#include "vga.h"
#include "bits.h"

void panic (const char *s) {
  puts("\r\nKERNEL PANIC: ");
  puts(s);
  halt();
}
