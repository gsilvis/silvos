#include "unwind.h"

#include "com.h"
#include "util.h"

extern const uint8_t _eh_frame_start[];
extern const uint8_t _eh_frame_end[];

/* DWARF data structs */
enum dwarf_rule {
  DW_UNDEFINED,
  DW_SAME,
  DW_OFFSET,
  DW_REG,
  DW_CFA,  /* reg + offset */
};

typedef struct {
  int32_t offset;
  int8_t reg;
  enum dwarf_rule rule;
} dwarf_loc;

typedef struct {
  uint64_t loc;
  dwarf_loc cfa;
  dwarf_loc registers[17];
} cfa_row;

typedef struct {
  uint64_t loc;
  uint64_t cfa;
  uint64_t registers[17];
  uint8_t regs_defined[3];  /* bit array */
} stack_frame;

typedef struct {
  uint64_t eh_data;
  uint64_t code_af;
  int64_t data_af;
  uint8_t return_addr_column;
  uint8_t aug_str_meaning;
  uint8_t fde_encoding;
  uint8_t lsda_encoding;
  cfa_row init_row;
} eh_cie;

typedef struct {
  const eh_cie *cie;
  uint64_t pc_begin;
  uint64_t pc_len;
  const uint8_t *instrs;
  uint64_t instrs_len;
} eh_fde;

/* This is a struct used all over the place in the file to parse the eh_frame
 * section. It's a string slice with a logical head at the beginning. */
typedef struct {
  const uint8_t *data;
  uint64_t remaining;
} head;

/* Splits a slice into two sub-slices.
 * Returns a head starting where 'in' used to start of length 'len', and moves
 * 'in' ahead by 'len' so they don't overlap. */
static head recurse(head *in, uint64_t len) {
  head subhead;
  subhead.data = in->data;
  subhead.remaining = len;
  in->data += len;
  in->remaining -= len;
  return subhead;
}

/* Convenience functions to parse ints of varying signedness and size.
 * The leb128 format is defined by DWARF. */
static int read_u8(head *in, uint8_t *out) {
  if (in->remaining < 1) {
    return -1;
  }
  *out = in->data[0];
  in->data++;
  in->remaining--;
  return 0;
}

static int read_char(head *in, char *out) {
  return read_u8(in, (uint8_t*)out);
}

static int read_u16(head *in, uint16_t *out) {
  if (in->remaining < 2) {
    return -1;
  }
  *out = in->data[1] | (in->data[0] << 8);
  in->data += 2;
  in->remaining -= 2;
  return 0;
}

static int read_u32(head *in, uint32_t *out) {
  if (in->remaining < 4) {
    return -1;
  }
  *out = 0;
  for (int i = 0; i < 4; ++i) {
    *out = *out | ((uint32_t)in->data[i]) << (i * 8);
  }
  in->data = in->data + 4;
  in->remaining -= 4;
  return 0;
}

static int read_u64(head *in, uint64_t *out) {
  if (in->remaining < 8) {
    return -1;
  }
  *out = 0;
  for (int i = 0; i < 8; ++i) {
    *out = *out | ((uint64_t)in->data[i]) << (i * 8);
  }
  in->data = in->data + 8;
  in->remaining -= 8;
  return 0;
}

static int read_uleb128(head *in, uint64_t *out) {
  *out = 0;

  int shift = 0;
  while (in->remaining > 0) {
    uint8_t c = in->data[0];
    in->data++;
    in->remaining--;
    *out = *out | ((uint64_t)c & 0x7F) << shift;
    shift += 7;
    if (!(c & 0x80)) {
      return 0;
    }
  }
  return -1;
}

static int read_sleb128(head *in, int64_t *out) {
  int read = 0;
  *out = 0;

  while (in->remaining > 0) {
    uint8_t c = in->data[0];
    in->data++;
    in->remaining--;
    *out = *out | ((uint64_t)c & 0x7F) << read;
    read += 7;
    if (!(c & 0x80)) {
      *out = (*out << (64 - read)) >> (64 - read);
      return 0;
    }
  }
  return -1;
}

/* Reads an encoded pointer value, whose size varies depending on ptr_encoding.
 * DWARF likes to use small pointer offsets from known locations rather than
 * full pointers. */
static int read_encoded_value(head *in, uint8_t ptr_encoding, int64_t *out) {
  uint32_t u32;
  int32_t s32;
  switch (ptr_encoding & 0x0F) {
    case 0:
      if (read_u64(in, (uint64_t *)out)) { return -1; }
      break;
    case 1:
      if (read_uleb128(in, (uint64_t *)out)) { return -1; }
      break;
    case 3:
      if (read_u32(in, &u32)) { return -1; }
      *out = u32;
      break;
    case 4:
      if (read_u64(in, (uint64_t *)out)) { return -1; }
      break;
    case 9:
      if (read_sleb128(in, out)) { return -1; }
      break;
    case 11:
      if (read_u32(in, (uint32_t *)&s32)) { return -1; }
      *out = s32;
      break;
    case 12:
      if (read_u64(in, (uint64_t *)out)) { return -1; }
      break;
    default:
      return -1;
  }
  return 0;
}

/* These functions all deal with the augmentation data, which has global config
 * such as "what format are we using for pointers". */
#define AUG_FLAG_LSDA     0x01
#define AUG_FLAG_FDE      0x02
#define AUG_FLAG_PERS     0x04
#define AUG_FLAG_AUG_DATA 0x40
#define AUG_FLAG_EH_DATA  0x80

static int read_aug_str(head *in, uint8_t *out) {
  char c;
  *out = 0;
  if (read_char(in, &c)) { return -1; }
  if (c == 'z') {
    *out = (*out) | AUG_FLAG_AUG_DATA;
  } else if (c == 'e') {
    if (read_char(in, &c)) { return -1; }
    if (c != 'h') { return -1; }
    *out = (*out) | AUG_FLAG_EH_DATA;
    if (read_char(in, &c)) { return -1; }
    if (c != 0) { return -1; }
    return 0;
  }

  while (!read_char(in, &c)) {
    if (c == 0) {
      return 0;
    }
    if (c == 'L') {
      *out = (*out) | AUG_FLAG_LSDA;
    } else if (c == 'R') {
      *out = (*out) | AUG_FLAG_FDE;
    } else if (c == 'P') {
      *out = (*out) | AUG_FLAG_PERS;
    }
  }
  return -1;
}

static int read_cie_aug_data(head *in, eh_cie *out) {
  uint64_t aug_data_len = 0;
  out->lsda_encoding = 0;
  out->fde_encoding = 0;
  if (read_uleb128(in, &aug_data_len)) { return -1; }
  head subhead = recurse(in, aug_data_len);
  if (out->aug_str_meaning & AUG_FLAG_PERS) {
    com_printf("BUG: personality section not supported");
    return -1;
  }
  if (out->aug_str_meaning & AUG_FLAG_LSDA) {
    if (read_u8(&subhead, &out->lsda_encoding)) { return -1; }
    com_printf("BUG: specifying LSDA encoding not supported");
  }
  if (out->aug_str_meaning & AUG_FLAG_FDE) {
    if (read_u8(&subhead, &out->fde_encoding)) { return -1; }
  }
  return 0;
}

static int read_fde_aug_data(head *in, eh_fde *out) {
  (void)out;
  uint64_t aug_data_len = 0;
  if (read_uleb128(in, &aug_data_len)) { return -1; }
  in->data += aug_data_len;
  in->remaining -= aug_data_len;
  if (aug_data_len > 0) {
    com_printf("BUG: Augmentation data for FDE not supported");
    return -1;
  }
  return 0;
}

/* Runs a single DWARF op parsed from the input head, using the context
 * provided by cie, and outputting the result in the given cfa_row. */
static int run_dwarf_op(head *in, const eh_cie *cie, cfa_row *row) {
  static cfa_row stack[32];
  static uint8_t stack_used = 0;
  uint8_t c;
  uint64_t reg;
  uint64_t off;
  uint8_t delta;
  uint16_t delta2;
  if (read_u8(in, &c)) { return -1; }
  switch (c >> 6) {
    case 0:
      /* not a primary op; see extended op table below */
      break;
    case 1:
      /* advance */
      delta = c & 0x3f;
      row->loc += delta * cie->code_af;
      return 0;
    case 2:
      /* offset */
      reg = c & 0x3f;
      if (read_uleb128(in, &off)) { return -1; }
      if (reg > 16) { return -1; }
      row->registers[reg].rule = DW_OFFSET;
      row->registers[reg].offset = off * cie->data_af;
      return 0;
    case 3:
      /* restore */
      reg = c & 0x3f;
      row->registers[reg] = cie->init_row.registers[reg];
      return 0;
  }
  switch (c) {
    case 0:
      /* nop */
      return 0;
    case 2:
      /* advance_loc1 */
      if (read_u8(in, &delta)) { return -1; }
      row->loc += delta * cie->code_af;
      return 0;
    case 3:
      /* advance_loc2 */
      if (read_u16(in, &delta2)) { return -1; }
      row->loc += delta2 * cie->code_af;
      return 0;
    case 10:
      /* remember state */
      if (stack_used >= 32) {
        com_printf("ERROR: Max unwind-handler stack depth exceeded.\n");
        return -1;
      }
      stack[stack_used++] = *row;
      return 0;
    case 11:
      /* restore state */
      if (stack_used == 0) {
        com_printf("BUG: Unwinder popped empty stack!\n");
        return -1;
      }
      /* Don't pop the code location T_T */
      stack[stack_used - 1].loc = row->loc;
      *row = stack[--stack_used];
      return 0;
    case 12:
      /* set cfa */
      if (read_uleb128(in, &reg)) { return -1; }
      if (read_uleb128(in, &off)) { return -1; }
      row->cfa.rule = DW_CFA;
      row->cfa.reg = reg;
      row->cfa.offset = off;
      return 0;
    case 13:
      /* set cfa_register */
      if (read_uleb128(in, &reg)) { return -1; }
      row->cfa.reg = reg;
      return 0;
    case 14:
      /* set cfa_offset */
      if (read_uleb128(in, &off)) { return -1; }
      row->cfa.offset = off;
      return 0;
    default:
      com_printf("BUG: unrecognized DWARF opcode %d\n", c);
      return -1;
  }
}

/* Reads the initial rules for a new DWARF context, assuming all others are
 * UNDEFINED. */
static int read_dwarf_init(head *in, eh_cie *cie) {
  for (int i = 0; i < 17; ++i) {
    cie->init_row.registers[i].rule = DW_UNDEFINED;
  }
  while (!run_dwarf_op(in, cie, &cie->init_row)) {
    if (in->remaining == 0) {
      return 0;
    }
  }
  return -1;
}

/* The cie is basically a preamble that we run before we start unwinding, it
 * largely exists to avoid lots of duplicated information for every frame
 * location. */
static int read_cie(head *in, eh_cie *out) {
  uint8_t version = 0;
  if (read_u8(in, &version) || version != 1) { return -1; }
  out->aug_str_meaning = 0;
  if (read_aug_str(in, &out->aug_str_meaning)) { return -1; }
  if (out->aug_str_meaning & AUG_FLAG_EH_DATA) {
    if (read_u64(in, &out->eh_data)) { return -1; }
  }
  if (read_uleb128(in, &out->code_af)) { return -1; }
  if (read_sleb128(in, &out->data_af)) { return -1; }
  if (read_u8(in, &out->return_addr_column)) { return -1; }
  if (out->aug_str_meaning & AUG_FLAG_AUG_DATA) {
    if (read_cie_aug_data(in, out)) { return -1; }
  }
  if (read_dwarf_init(in, out)) { return -1; }
  return 0;
}

/* Frame Description Entity: basically defines a region of code and what rules
 * let you unwind from that region. */
static int read_fde(head *in, eh_fde *out) {
  int64_t raw_pc_begin;
  uint64_t origin = (uint64_t) in->data;
  if (read_encoded_value(in, out->cie->fde_encoding, &raw_pc_begin)) { return -1; }
  if (read_encoded_value(in, out->cie->fde_encoding, (int64_t *)&out->pc_len)) { return -1; }
  switch ((out->cie->fde_encoding >> 4) & 0x3) {
    case 0:
      out->pc_begin = raw_pc_begin;
      break;
    case 1:
      out->pc_begin = raw_pc_begin + origin;
      break;
    case 3:
      com_printf("No eh_frame_hdr yet.\n");
      return -1;
    default:
      return -1;
  }
  if (out->cie->aug_str_meaning & AUG_FLAG_AUG_DATA) {
    if (read_fde_aug_data(in, out)) { return -1; }
  }
  out->instrs = in->data;
  out->instrs_len = in->remaining;
  return 0;
}

/* Applies a definition to get the current value of a register from a DWARF rule */
static int get_def(const stack_frame *base, const cfa_row *row, int reg_or_cfa, uint64_t *out) {
  dwarf_loc loc = (reg_or_cfa == -1) ? row->cfa : row->registers[reg_or_cfa];
  switch (loc.rule) {
    case DW_UNDEFINED:
      return 0;
      break;
    case DW_SAME:
      if (reg_or_cfa == -1) {
        com_printf("BUG: Confusing register definition: CFA = SAME\n");
        return -1;
      }
      if (!bit_array_get(base->regs_defined, reg_or_cfa)) {
        com_printf("ERROR: Can't use rule SAME on undefined register %d\n", reg_or_cfa);
        return -1;
      }
      *out = base->registers[reg_or_cfa];
      break;
    case DW_OFFSET:
      /* ! LEBENSGEFAHR ! */
      *out = *(uint64_t *)(base->cfa + loc.offset);
      break;
    case DW_REG:
      *out = base->registers[loc.reg];
      break;
    case DW_CFA:
      *out = base->registers[loc.reg] + loc.offset;
      break;
  }
  return 1;
}

static int unwind_frame(stack_frame *base, const cfa_row *row, stack_frame *out) {
  int ret = get_def(base, row, -1, &base->cfa);
  if (ret < 0) { return ret; }
  if (ret == 0) {
    com_printf("ERROR: Need definition for CFA!\n");
    return -1;
  }

  for (int i = 0; i < 17; ++i) {
    int ret = get_def(base, row, i, &out->registers[i]);
    if (ret < 0) {
      return -1;
    }
    bit_array_set(out->regs_defined, i, ret);
  }
  out->registers[7] = base->cfa;
  bit_array_set(out->regs_defined, 7, ret);
  return 0;
}

/* Searches for the right FDE for a given program location and returns both that
 * and the CIE for that FDE while we're at it. */
static int find_fde(uint64_t rip, eh_cie* cie_out, eh_fde* fde_out) {
  head read_head;
  read_head.data = _eh_frame_start;
  read_head.remaining = _eh_frame_end - _eh_frame_start;

  eh_cie current_cie;
  memset(&current_cie, 0, sizeof(eh_cie));
  while (read_head.remaining > 0) {
    uint32_t short_length;
    uint64_t long_length;
    if (read_u32(&read_head, &short_length)) { return -1; }
    if (short_length == (uint32_t)-1) {
      if (read_u64(&read_head, &long_length)) { return -1; }
    } else {
      long_length = short_length;
    }

    if (read_head.remaining < long_length) { return -1; }

    head subhead = recurse(&read_head, long_length);

    /* tells us if we're about to read a CIE or an FDE */
    uint32_t id;
    if (read_u32(&subhead, &id)) { return -1; }
    if (id == 0) {
      if (read_cie(&subhead, &current_cie)) { return -1; }
      continue;
    }
    eh_fde fde;
    memset(&fde, 0, sizeof(eh_fde));
    fde.cie = &current_cie;
    if (read_fde(&subhead, &fde)) { return -1; }
    if (rip >= fde.pc_begin && rip - fde.pc_begin < fde.pc_len) {
      *cie_out = current_cie;
      fde.cie = cie_out;
      *fde_out = fde;
      return 0;
    }
  }
  return 1;
}

/* self-test: let's see if we can just *parse* everything in eh_frame */
void test_parse_eh_frame() {
  eh_cie cie;
  eh_fde fde;
  find_fde((uint64_t)-1, &cie, &fde);
}

void gen_backtrace(uint64_t rsp, uint64_t rip, uint64_t rbp) {
  /* Set initial register state. AFAIK we never need anything other than rbp and rsp,
   * but that could change in the future. */
  stack_frame bot_frame;
  memset(&bot_frame, 0, sizeof(bot_frame));
  bot_frame.registers[6] = rbp;
  bot_frame.registers[7] = rsp;
  /* On our architecture, the CFA (basically, the logical frame pointer) is exactly
   * the same as rsp. */
  bot_frame.cfa = rsp;
  bot_frame.loc = rip;
  bit_array_set(bot_frame.regs_defined, 6, 1);
  bit_array_set(bot_frame.regs_defined, 7, 1);

  int frame_no = 0;
  com_printf("Stack trace: \n");
  while (1) {
    com_printf("%d: %p\n", frame_no, rip);
    ++frame_no;
    eh_cie cie;
    eh_fde fde;

    int ret = find_fde(rip, &cie, &fde);
    if (ret == 1) {
      com_printf("(Top of stack frame)\n");
      return;
    } else if (ret) {
      com_printf("Error encountered! Sorry.\n");
      return;
    }

    head in;
    in.data = fde.instrs;
    in.remaining = fde.instrs_len;

    cfa_row row = fde.cie->init_row;
    row.loc = fde.pc_begin;
    while (row.loc <= rip) {
      if (run_dwarf_op(&in, fde.cie, &row)) { return; }
      if (in.remaining == 0) {
        break;
      }
    }
    stack_frame next_frame;
    if (unwind_frame(&bot_frame, &row, &next_frame)) {
      com_printf("(Failed to unwind next frame, giving up)\n");
      return;
    }
    bot_frame = next_frame;
    rip = next_frame.registers[16];
  }
}
