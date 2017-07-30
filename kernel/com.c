#include "com.h"

#include "pagefault.h"
#include "printf.h"
#include "threads.h"
#include "util.h"

#include <stdarg.h>
#include <stddef.h>

static const uint16_t PORT = 0x3f8;

void com_initialize (void) {
  outb(PORT + 1, 0x00);    /* Disable all interrupts */
  outb(PORT + 3, 0x80);    /* Enable DLAB (set baud rate divisor) */
  outb(PORT + 0, 0x03);    /* Set divisor to 3 (lo byte) 38400 baud */
  outb(PORT + 1, 0x00);    /*                  (hi byte) */
  outb(PORT + 3, 0x03);    /* 8 bits, no parity, one stop bit */
  outb(PORT + 4, 0x03);    /* RTS/DSR set */
}

void com_putch (char c) {
  while (0 == (inb(PORT + 5) & 0x20));
  outb(PORT, c);
}

void com_puts (const char *text) {
  for (int i = 0; text[i]; ++i) {
    com_putch(text[i]);
  }
}

uint32_t com_debug_thread (char *text, uint32_t len) {
  /* TODO: check that user text is nice (doesn't contain control characters,
   * for intance) */
  char tmp[61];
  if (len > 60) {
    len = 60;
  }
  if (copy_from_user(&tmp[0], text, len)) {
    return 0;
  }
  tmp[len] = 0;
  com_printf("THREAD 0x%02X: %s\n", running_tcb->thread_id, tmp);

  return len;
}

int com_printf (const char *fmt, ...) {
  int ret;
  va_list argp;
  va_start(argp, fmt);
  ret = kvprintf(com_putch, fmt, argp);
  va_end(argp);
  return ret;
}
