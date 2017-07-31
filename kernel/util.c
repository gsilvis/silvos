#include "util.h"

#include "threads.h"
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

uint64_t strlen (const char *s) {
  uint64_t i = 0;
  while (s[i]) i++;
  return i;
}

void __attribute__ ((noreturn)) qemu_debug_shutdown (void) {
  /* This works, as long as you include the following argument to QEMU:
   *    -device isa-debug-exit,iobase=0xf4,iosize=0x04
   * This will not work on any real hardware.
   */
  outb(0xf4, 0x00);
  panic("Shutdown failed!");
}

static int panic_count = 0;
void __attribute__ ((noreturn)) panic (const char *s) {
  panic_count++;
  if (panic_count == 1) {
    vga_printf("\r\nKERNEL PANIC: %s\r\n", s);
    vga_printf("Generating backtrace to COM...\r\n");
    com_print_backtrace();
  } else if (panic_count == 2) {
    /* Double panic! No time for anything fancy. */
    puts("\r\nDOUBLE KERNEL PANIC: ");
    puts(s);
  } else if (panic_count == 3) {
    puts("\r\nTRIPLE KERNEL PANIC!");
  } else {
    /* Seriously? I give up. */
  }
  while (1) {
    hlt();
  }
}

void blab (void) {
  puts("\r\n HEYYYYYYYYY!!!!!!!!!!!!!!!!!!\r\n");
}
