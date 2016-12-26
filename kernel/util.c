#include "util.h"

#include "vga.h"

#include <stdint.h>
#include <stddef.h>

void memset (void *ptr, char byte, size_t count) {
  char *p = ptr;
  for (unsigned int i = 0; i < count; i++) {
    p[i] = byte;
  }
}

void memcpy (void *dest, const void *src, size_t count) {
  char *d = dest;
  const char *s = src;
  for (unsigned int i = 0; i < count; i++) {
    d[i] = s[i];
  }
}

int strncmp (const char *a, const char *b, size_t n) {
  for (unsigned int i = 0; i < n; i++) {
    if (a[i] < b[i]) {
      return -1;
    } else if (a[i] > b[i]) {
      return 1;
    }
  }
  return 0;
}

void __attribute__ ((noreturn)) qemu_debug_shutdown (void) {
  /* This works, as long as you include the following argument to QEMU:
   *    -device isa-debug-exit,iobase=0xf4,iosize=0x04
   * This will not work on any real hardware.
   */
  outb(0xf4, 0x00);
  panic("Shutdown failed!");
}

void __attribute__ ((noreturn)) panic (const char *s) {
  puts("\r\nKERNEL PANIC: ");
  puts(s);
  while (1) {
    hlt();
  }
}

void blab (void) {
  puts("\r\n HEYYYYYYYYY!!!!!!!!!!!!!!!!!!\r\n");
}