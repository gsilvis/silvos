#include "printf.h"

#include "com.h"
#include "util.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define emit(c) do { com_putch(c); ++ret; } while(0)
#define emits(s) do { const char *_c = (s); \
                      while (*_c) { emit(*(_c++)); } } while(0)

#define FLAGS_LEFT_JUST 0x01
#define FLAGS_USE_PLUS  0x02
#define FLAGS_USE_SPACE 0x04
#define FLAGS_ZERO_PAD  0x08

enum arg_size {
  SIZE_SHORT,
  SIZE_DEFAULT,
  SIZE_LONG,
};

typedef struct {
  char fmt;
  enum arg_size size;
  uint8_t bitlength;
  uint64_t min_width;
  int flags;
  union {
    uint64_t uarg;
    int64_t sarg;
    char charg;
    const char *strarg;
  } arg;
  /* Long enough for decimal 2**64-1 + NUL */
  char work[22];
  /* Contains NULL, " ", "+", "-", or "0x" depending on flags/arg sign/fmt */
  const char* prefix;
} format;

/* Returns number of consumed characters. If this function reaches a null byte,
 * str[return_value] will be zero. Otherwise, str[return_value] is the first
 * character after the format string blob. */
static int parse_fmt_string(const char *fmt, format *fspec) {
  int i = 0;
  char c = fmt[i];
  fspec->flags = 0;
  while (c) {
    if (c == '-') {
      fspec->flags |= FLAGS_LEFT_JUST;
    } else if (c == '+') {
      fspec->flags |= FLAGS_USE_PLUS;
    } else if (c == '0') {
      fspec->flags |= FLAGS_ZERO_PAD;
    } else if (c == ' ') {
      fspec->flags |= FLAGS_USE_SPACE;
    } else {
      break;
    }
    c = fmt[++i];
  }
  if (c == 0)  return i;

  fspec->min_width = 0;
  while (c) {
    if ('0' <= c && c <= '9') {
      fspec->min_width = fspec->min_width * 10 + (c - '0');
      c = fmt[++i];
    } else {
      break;
    }
  }
  if (c == 0)  return i;

  fspec->size = SIZE_DEFAULT;
  if (c == 'h') {
    fspec->size = SIZE_SHORT;
    c = fmt[++i];
  } else if (c == 'l') {
    fspec->size = SIZE_LONG;
    c = fmt[++i];
  }
  if (c == 0)  return i;

  fspec->fmt = c;
  ++i;
  return i;
}

/* Consumes the approprite amount from argp to read the argument specified by
 * fspec->fmt, and sets some other state values based on the fmt and, for
 * %d and %i, the sign of the argument that was read. */
static int read_arg(format *fspec, va_list argp) {
  fspec->prefix = NULL;
  switch (fspec->fmt) {
    case 'x':
    case 'X':
    case 'o':
    case 'u':
      switch (fspec->size) {
        case SIZE_SHORT:
          fspec->bitlength = 16;
          fspec->arg.uarg = (uint16_t) va_arg(argp, int);
          break;
        case SIZE_DEFAULT:
          fspec->bitlength = 32;
          fspec->arg.uarg = va_arg(argp, uint32_t);
          break;
        case SIZE_LONG:
          fspec->bitlength = 64;
          fspec->arg.uarg = va_arg(argp, uint64_t);
          break;
      }
      break;
    case 'd':
    case 'i':
      switch (fspec->size) {
        case SIZE_SHORT:
          fspec->bitlength = 16;
          fspec->arg.sarg = (int16_t) va_arg(argp, int);
          break;
        case SIZE_DEFAULT:
          fspec->bitlength = 32;
          fspec->arg.sarg = va_arg(argp, int32_t);
          break;
        case SIZE_LONG:
          fspec->bitlength = 64;
          fspec->arg.sarg = va_arg(argp, int64_t);
          break;
      }
      if (fspec->arg.sarg < 0) {
        fspec->prefix = "-";
      } else if (fspec->flags & FLAGS_USE_PLUS) {
        fspec->prefix = "+";
      } else if (fspec->flags & FLAGS_USE_SPACE) {
        fspec->prefix = " ";
      }
      break;
    case 'p':
      fspec->bitlength = 64;
      fspec->arg.uarg = va_arg(argp, uint64_t);
      fspec->prefix = "0x";
      break;
    case 'c':
      fspec->arg.charg = (char) va_arg(argp, int);
      break;
    case 's':
      fspec->arg.strarg = va_arg(argp, const char*);
      break;
    default:
      return -1;
  }

  return 0;
}

/* Writes the representation of the contents of fspec into its work buffer,
 * then returns a pointer into somewhere inside the work buffer that is the
 * start of the serialized output.
 * Returns NULL on failure, which shouldn't happen */
static const char *serialize(format *fspec) {
  const char hex_lower[16] = "0123456789abcdef";
  /* This alphabet works for decimal, octal, and uppercase hex */
  const char hex_upper[16] = "0123456789ABCDEF";
  const char *alphabet = hex_upper;

  uint64_t arg;
  int mask_step;
  switch (fspec->fmt) {
    case 's':
      return fspec->arg.strarg;
    case 'c':
      fspec->work[0] = fspec->arg.charg;
      fspec->work[1] = 0;
      return fspec->work;
    case 'x':
      alphabet = hex_lower;
    case 'p':
    case 'X':
      mask_step = 16;
      arg = fspec->arg.uarg;
      break;
    case 'o':
      mask_step = 8;
      arg = fspec->arg.uarg;
      break;
    case 'u':
      mask_step = 10;
      arg = fspec->arg.uarg;
      break;
    case 'd':
    case 'i':
      mask_step = 10;
      if (fspec->arg.sarg < 0) {
        arg = (uint64_t)(-fspec->arg.sarg);
      } else {
        arg = (uint64_t)fspec->arg.sarg;
      }
      break;
    default:
      panic("Unexpected format specified found after parsing format string!");
  }

  int index = sizeof(fspec->work) - 1;
  fspec->work[index] = 0;
  do {
    index--;
    fspec->work[index] = alphabet[(arg % mask_step)];
    arg = arg / mask_step;
  } while (arg > 0);

  return fspec->work + index;
}

int printf (const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  int ret = 0;
  int i = 0;

  while (fmt[i]) {
    char c = fmt[i++];
    if (c != '%') {
      emit(c);
      continue;
    }

    format fspec;
    i += parse_fmt_string(fmt + i, &fspec);
    if (!fmt[i])  return ret;

    if (fspec.fmt == '%') {
      my_putch(c);
      ret++;
      continue;
    }

    read_arg(&fspec, argp);
    const char *buffer = serialize(&fspec);

    uint64_t len = strlen(buffer);
    char padchar = ' ';
    if ((fspec.flags & FLAGS_ZERO_PAD) && !(c == 's' || c == 'c')) {
      padchar = '0';
    }
    if (fspec.prefix) {
      len += strlen(fspec.prefix);
      /* For zero-padding the prefix goes before padding; otherwise after. */
      if (padchar == '0') {
        emits(fspec.prefix);
      }
    }
    if (!(fspec.flags & FLAGS_LEFT_JUST)) {
      for (uint64_t j = len; j < fspec.min_width; ++j) {
        emit(padchar);
      }
    }
    if (fspec.prefix && padchar == ' ') {
      emits(fspec.prefix);
    }
    for (uint64_t j = 0; buffer[j]; ++j) {
      emit(buffer[j]);
    }
    if (fspec.flags & FLAGS_LEFT_JUST) {
      for (uint64_t j = len; j < fspec.min_width; ++j) {
        emit(' ');
      }
    }
  }
  return ret;
}

