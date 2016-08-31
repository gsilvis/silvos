#include "userland.h"

void main (void) {
  for (int i = 0; i < 100; i++) {
    putch(getch());
  }
}
