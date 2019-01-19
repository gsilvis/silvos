#include "apic.h"

#include <stdint.h>
#include <stddef.h>

#include "acpi.h"
#include "com.h"
#include "hpet.h"
#include "idt.h"
#include "memory-map.h"
#include "util.h"
#include "vga.h"

/* ACPI registers contain 4 bytes of data but use 16 bytes of space.  Writing
 * to or reading from the upper 12 bytes is always undefined behavior.  Reading
 * or writing less than an entire 32-bit register at once is also undefined
 * behavior.  So is using any FP, MMX, SSE etc instruction to read or write
 * from them.  Be careful. */
struct APICRegister {
  uint32_t volatile val;
  uint32_t unused[3];
}  __attribute__ ((packed));

/* 1024 bytes long (64 32-bit registers).  They are grouped into sets of 8
 * registers to make the exact size a little clearer. */
struct APIC {
  struct APICRegister reserved_1[2];
  struct APICRegister local_apic_id;
  struct APICRegister local_apic_version_id;
  struct APICRegister reserved_2[4];

  struct APICRegister task_priority;
  struct APICRegister arbitration_priority;
  struct APICRegister processor_priority;
  struct APICRegister end_of_interrupt;
  struct APICRegister remote_read;
  struct APICRegister logical_destination;
  struct APICRegister destination_format;
  struct APICRegister spurious_interrupt_vector;

  struct APICRegister in_service[8];

  struct APICRegister trigger_mode[8];

  struct APICRegister interrupt_request[8];

  struct APICRegister error_status;
  struct APICRegister reserved_3[6];
  struct APICRegister lvt_cmci;

  struct APICRegister interrupt_command[2];
  struct APICRegister lvt_timer;
  struct APICRegister lvt_thermal_sensor;
  struct APICRegister lvt_performance_monitoring_counters;
  struct APICRegister lvt_lint0;
  struct APICRegister lvt_lint1;
  struct APICRegister lvt_error;

  struct APICRegister timer_initial_count;
  struct APICRegister timer_current_count;
  struct APICRegister reserved_4[4];
  struct APICRegister timer_divide_configuration;
  struct APICRegister reserved_5[1];
} __attribute__ ((packed));

static const uint32_t APIC_SPURIOUS_VECTOR_ENABLE_APIC = 0x00000100;

struct IOAPIC {
  uint32_t volatile reg;
  uint32_t unused[3];
  uint32_t volatile data;
} __attribute ((packed));

static const uint32_t IOAPIC_REG_ID             = 0x0000;
static const uint32_t IOAPIC_REG_VERSION        = 0x0001;
static const uint32_t IOAPIC_REG_ARBITRATION_ID = 0x0002;

static inline uint32_t IOAPIC_RED_TBL_LOW(uint8_t line) {
  return 0x10 + line*2;
}
static inline uint32_t IOAPIC_RED_TBL_HIGH(uint8_t line) {
  return 0x10 + line*2 + 1;
}

static struct APIC *apic;
static struct IOAPIC *ioapic;

#define NUM_ISA_LINES 16

struct isa_irq_remapping {
  uint8_t isa_line;
  uint8_t polarity_is_low; /* Normally ISA is active-high */
  uint8_t trigger_is_level; /* Normally ISA is edge-triggered */
  uint32_t remapped_apic_line;
};
static struct isa_irq_remapping remaps[NUM_ISA_LINES];
static uint8_t nremaps = 0;

static void ioapic_remap_irq (
    uint8_t isa_line,
    uint32_t remapped_apic_line,
    uint8_t polarity_is_low,
    uint8_t trigger_is_level) {
  if (nremaps >= NUM_ISA_LINES) {
    panic("Somehow found more ISA remaps than exist ISA lines.");
  }
  remaps[nremaps].isa_line = isa_line;
  remaps[nremaps].remapped_apic_line = remapped_apic_line;
  remaps[nremaps].polarity_is_low = polarity_is_low;
  remaps[nremaps].trigger_is_level = trigger_is_level;
  nremaps++;
}

enum MadtEntryType {
  MADT_ENTRY_LOCAL_APIC = 0,
  MADT_ENTRY_IO_APIC = 1,
  MADT_ENTRY_ISA_OVERRIDE = 2,
  MADT_ENTRY_NMI_OVERRIDE = 4,
};

/* Returns the size of the entry */
static uint8_t analyze_madt_entry (uint8_t *entry) {
  uint16_t *entry_16 = (uint16_t *)entry;
  uint32_t *entry_32 = (uint32_t *)entry;
  uint8_t entry_type = entry[0];
  uint8_t entry_size = entry[1];
  /* TODO: Print bytes with the hh specifier once it's supported. */
  switch (entry_type) {
    case MADT_ENTRY_LOCAL_APIC:
      vga_printf("CPU-Local APIC for CPU %hd with ID %hd\r\n", entry[2], entry[3]);
      if (entry_32[1] == 0) {
        vga_printf("THIS CPU IS DISABLED!?\r\n");
      }
      break;
    case MADT_ENTRY_IO_APIC:
      vga_printf("IO APIC %hd at 0x%X with interrupt base %d\r\n", entry[2], entry_32[1], entry_32[2]);
      ioapic = (struct IOAPIC*)phys_to_virt((uint64_t)entry_32[1]);
      break;
    case MADT_ENTRY_ISA_OVERRIDE: {
      uint8_t isa_line = entry[3];
      uint32_t remapped_apic_line = entry_32[1];
      uint16_t flags = entry_16[4];
      uint8_t polarity_is_low = ((flags & 0x03) == 0x01);
      uint8_t trigger_is_level = ((flags & 0x0C) == 0x11);
      vga_printf("ISA IRQ Override from %hd to %d with flags 0x%hX (low=%d level=%d)\r\n", isa_line, remapped_apic_line, flags, polarity_is_low, trigger_is_level);
      ioapic_remap_irq(isa_line, remapped_apic_line, polarity_is_low, trigger_is_level);
      break;
    }
    case MADT_ENTRY_NMI_OVERRIDE: {
      uint16_t flags = ((uint16_t)entry[3]) * 256 + (uint16_t)entry[4];
      if (entry[2] == 255) {
        vga_printf("Local APIC NMI structure for all CPUs: flags 0x%hX, NMI is line %hd\r\n", flags, entry[5]);
      } else {
        vga_printf("Local APIC NMI structure for CPU %hd: flags 0x%hX, NMI is line %hd\r\n", entry[2], flags, entry[5]);
      }
      break;
    }
    default:
      vga_printf("Unimplemented MADT entry with type: %hd\r\n", entry_type);
      break;
  }
  return entry_size;
}

void apic_init (void) {
  vga_printf("CPU-Local APIC is at 0x%X\r\n", madt->local_interrupt_controller_address);
  apic = (struct APIC*)phys_to_virt((uint64_t)madt->local_interrupt_controller_address);

  const uint32_t total_length = madt->h.Length;
  uint8_t *slice = (uint8_t *)madt;
  uint32_t offset = sizeof(struct MADT);
  while (offset < total_length) {
    offset += analyze_madt_entry(&slice[offset]);
  }

  if (!ioapic) {
    panic("No IOAPIC found!");
  }

  apic->spurious_interrupt_vector.val = 0xFF | APIC_SPURIOUS_VECTOR_ENABLE_APIC;
}

void ap_apic_init (void) {
  apic->spurious_interrupt_vector.val = 0xFF | APIC_SPURIOUS_VECTOR_ENABLE_APIC;
}

void apic_eoi (void) {
  apic->end_of_interrupt.val = 0;
}

/* Format for APIC ICR registers and for IOAPIC redirection table registers. */

static const uint32_t APIC_ICR_LOW_DELIVERY_MODE_FIXED          = 0x00000000;
static const uint32_t APIC_ICR_LOW_DELIVERY_MODE_LOW_PRIORITY   = 0x00000100;
static const uint32_t APIC_ICR_LOW_DELIVERY_MODE_SMI            = 0x00000200;
static const uint32_t APIC_ICR_LOW_DELIVERY_MODE_NMI            = 0x00000400;
static const uint32_t APIC_ICR_LOW_DELIVERY_MODE_INIT           = 0x00000500;
static const uint32_t APIC_ICR_LOW_DELIVERY_MODE_START_UP       = 0x00000600;

static const uint32_t APIC_ICR_LOW_DESTINATION_MODE_LOGICAL     = 0x00000800;

static const uint32_t APIC_ICR_LOW_DELIVERY_STATUS_SEND_PENDING = 0x00001000;

static const uint32_t APIC_ICR_LOW_PIN_POLARITY_ACTIVE_LOW      = 0x00002000;

static const uint32_t APIC_ICR_LOW_LEVEL_ASSERT                 = 0x00004000;

static const uint32_t APIC_ICR_LOW_TRIGGER_LEVEL                = 0x00008000;

static const uint32_t APIC_ICR_LOW_MASK_LINE                    = 0x00010000;

static const uint32_t APIC_ICR_LOW_DEST_SHORTHAND_SELF          = 0x00040000;
static const uint32_t APIC_ICR_LOW_DEST_SHORTHAND_ALL_AND_SELF  = 0x00080000;
static const uint32_t APIC_ICR_LOW_DEST_SHORTHAND_ALL_OTHERS    = 0x000C0000;

static inline uint32_t APIC_ICR_HIGH_DESTINATION(uint32_t dest) {
  return dest << 24;
}

static void write_ioapic_redirection (uint32_t line, uint32_t low_flags, uint32_t high_flags) {
  ioapic->reg = IOAPIC_RED_TBL_LOW(line);
  ioapic->data = low_flags;
  ioapic->reg = IOAPIC_RED_TBL_HIGH(line);
  ioapic->data = high_flags;
}

static struct isa_irq_remapping find_isa_mapping (uint8_t isa_line) {
  /* Search for an explicit remapping of our ISA line. */
  for (uint8_t i = 0; i < nremaps; i++) {
    if (remaps[i].isa_line == isa_line) {
      return remaps[i];
    }
  }
  /* There was no explicit remapping; check if it's safe to put in a default
   * route to interrupt line 'isa_line + 64'. */
  for (uint8_t i = 0; i < nremaps; i++) {
    if (remaps[i].remapped_apic_line == isa_line) {
      /* ISA line 'i' is already using our line; so, our line has been eaten,
       * and can't be used. */
      panic("Tried to use ISA line with no mapping.");
    }
  }
  /* No explicit remapping, and it's safe to use the default mapping, so do so. */
  struct isa_irq_remapping result = {
    .isa_line = isa_line,
    .polarity_is_low = 0,
    .trigger_is_level = 0,
    .remapped_apic_line = isa_line,
  };
  return result;
}

static unsigned char interrupt_vector_for_isa_line (uint8_t isa_line) {
  /* Map ISA lines to interrupt vectors 64-80; we could use a more complex
   * allocation scheme, but there's no need for now. */
  if (isa_line >= 16) {
    panic("Invalid isa_line");
  }
  return isa_line + 64;
}

void register_isa_isr (uint8_t isa_line, void (*handler)(void)) {
  struct isa_irq_remapping mapping = find_isa_mapping(isa_line);
  unsigned char interrupt_vector = interrupt_vector_for_isa_line(isa_line);
  uint32_t low_flags = interrupt_vector
    | (mapping.polarity_is_low ? APIC_ICR_LOW_PIN_POLARITY_ACTIVE_LOW : 0)
    | (mapping.trigger_is_level ? APIC_ICR_LOW_TRIGGER_LEVEL : 0);
  uint32_t high_flags = APIC_ICR_HIGH_DESTINATION(0);
  register_isr(interrupt_vector, handler, 0);
  write_ioapic_redirection(mapping.remapped_apic_line, low_flags, high_flags);
}
