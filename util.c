#include "vga.h"

void panic (const char *s) {
  puts("\r\nKERNEL PANIC: ");
  puts(s);
  __asm__("hlt");
}
