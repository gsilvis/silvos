#include "vga.h"
#include "bits.h"

void memset (char *ptr, char byte, unsigned int count) {
  for (unsigned int i = 0; i < count; i++) {
    ptr[i] = byte;
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

int read_key (void) {
  return !(0x80 & inb(0x60));
}
