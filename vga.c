#include "vga.h"

#define VGA 0xB8000

typedef struct {
  char c;
  unsigned char color;
} vga_screen[25][80];

vga_screen *myvga = (vga_screen*) VGA;

void putc (char c) {
  static unsigned int x = 0;
  static unsigned int y = 0;
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

void puti (unsigned int i) {
  char d = i % 10 + '0';
  unsigned int rest = i / 10;
  if (rest) {
    puti(rest);
  }
  putc(d);
}
