#include "vga.h"
#include "bits.h"

void __attribute__ ((noreturn)) panic (const char *s) {
  cli();
  puts("\r\nKERNEL PANIC: ");
  puts(s);
  while (1) {
    hlt();
  }
}
