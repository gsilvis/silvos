#include "userland.h"

void main (void) {
  while (1) {
//    putch('B');
    __asm__ volatile("mov $50000000,%ecx; \
                      L1:  loop L1");
  }
}
