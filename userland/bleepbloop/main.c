#include "userland.h"

void main (void) {
  static int k = 0;
  unsigned short *my_page = (unsigned short *)0xC0000000;
  palloc(my_page);
  getch();
  while (1) {
    for (int i = 0; i < 2048; i+=2) {
      unsigned char lval = ((k*(k>>8|k>>9)&46&k>>8))^((k&k>>9)|k>>3)^((k>>1)*(k>>8&k>>12)&101&k>>8);
      unsigned short val = lval;
      val = val * 32;
      my_page[i] = val;
      my_page[i+1] = val;
      k++;
    }
    audio_out(my_page);
  }
}
