#include "userland.h"

void main (void) {
  char a = getch();
  float c = (float) a;
  while (c > 0.89) {
    putch('!');
    c /= 1.34;
  }
  char *str = "I'M REALLY ANNOYING";
  for (unsigned int i = 0; i < 19; i++) {
    putch(str[i]);
    __asm__ volatile("mov $50000000,%%ecx; \
                      L1:  loop L1" ::: "%ecx");
  }
}
