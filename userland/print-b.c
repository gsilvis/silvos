#include "userland.h"

void main (void) {
  char *str = "I'M REALLY ANNOYING";
  for (unsigned int i = 0; i < 19; i++) {
    putch(str[i]);
    __asm__ volatile("mov $50000000,%%ecx; \
                      L1:  loop L1" ::: "%ecx");
  }
}
