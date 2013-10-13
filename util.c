#include "vga.h"
#include "bits.h"

void __attribute__ ((noreturn)) panic (const char *s) {
  puts("\r\nKERNEL PANIC: ");
  puts(s);
  halt();
}
