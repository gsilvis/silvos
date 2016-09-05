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

void __attribute__ ((noreturn)) panic (const char *s) {
  cli();
  puts("\r\nKERNEL PANIC: ");
  puts(s);
  while (1) {
    hlt();
  }
}

void blab (void) {
  puts("\r\n HEYYYYYYYYY!!!!!!!!!!!!!!!!!!\r\n");
}
