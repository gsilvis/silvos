#include "userland.h"

#include <stdint.h>

static const uint16_t BASE_PORT = 0x3f8;
static const uint16_t DUMB_PORT = 0x3f8 + 0x5;

static void outb (uint16_t port, uint8_t data) {
  __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static uint8_t inb (uint16_t port) {
  uint8_t data;
  __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
  return data;
}

static void com_putch (char c) {
  while (0 == (inb(DUMB_PORT) & 0x20));
  outb(BASE_PORT, c);
}

static void com_puts (const char *text) {
  for (int i = 0; text[i]; ++i) {
    com_putch(text[i]);
  }
}

void main() {
  if (get_ioport(BASE_PORT)) {
    debug("Failed to get base port");
  }
  if (get_ioport(DUMB_PORT)) {
    debug("Failed to get dumb port");
  }
  com_puts("LOOK AT ME I CAN PRINT DIRECTLY!\n");
}
