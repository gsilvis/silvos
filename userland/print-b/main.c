#include "userland.h"

void main (void) {
  char a = getch();
  float c = (float) a;
  while (c > 0.89) {
    putch('!');
    c /= 1.34;
  }
  putch('\r');
  putch('\n');
  putch('x');
  putch(':');
  char j[512];
  if (read(0, &j[0])) {
    return;
  }
  for (int i = 0; (i < 512) && j[i]; i++) {
    putch(j[i]);
  }
  j[0] = 'H';
  j[1] = 'e';
  j[2] = 'l';
  j[3] = 'l';
  j[4] = 'o';
  j[5] = '!';
  j[6] = '!';
  j[7] = '\0';
  if (write(0, &j[0])) {
    return;
  }
  putch('\r');
  putch('\n');
  char *str = "I'M REALLY ANNOYING";
  for (unsigned int i = 0; i < 19; i++) {
    putch(str[i]);
    __asm__ volatile("mov $50000000,%%ecx; \
                      L1:  loop L1" ::: "%ecx");
  }
}
