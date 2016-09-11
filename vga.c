#include "vga.h"

#include <stdint.h>

#define VGA 0xB8000

typedef struct {
  char c;
  uint8_t color;
} vga_screen[25][80];

vga_screen *myvga = (vga_screen*) VGA;

void putc (char c) {
  static int x = 0;
  static int y = 0;
  switch (c) {
  case '\r':
    x = 0;
    break;
  case '\n':
    y++;
    break;
  case '\177':
    if (x > 0) {
      x--;
    } else {
      x = 79; /* Go to end of previous line */
      if (y > 0) {
        y--;
      } else {
        y = 24; /* Go back to bottom of screen. */
      }
    }
    (*myvga)[y][x].c = ' ';
    break;
  default:
    (*myvga)[y][x].c = c;
    x++;
    if (x == 80) {
      x = 0; /* Go to beginning of next line */
      y++;
    }
    break;
  }
  if (y == 25) {
    y = 0; /* Go back to top of screen. */
  }
}

void clear_screen (void) {
  for (int i = 0; i < 25*80; i++) {
    putc(' ');
  }
}

void puts (const char *s) {
  for (const char *cp = s; *cp; cp++) {
    putc(*cp);
  }
}

void puti (uint32_t i) {
  char d = i % 10 + '0';
  uint32_t rest = i / 10;
  if (rest) {
    puti(rest);
  }
  putc(d);
}

void put_byte (uint8_t d) {
  const char *out = "0123456789ABCDEF";
  putc(out[0x0F & (d >> 4)]);
  putc(out[0x0F & (d)]);
}

void put_short (uint16_t d) {
  put_byte((uint8_t)(d >> 8));
  put_byte((uint8_t)(d));
}

void put_int (uint32_t d) {
  put_short((uint16_t)(d >> 16));
  put_short((uint16_t)(d));
}
