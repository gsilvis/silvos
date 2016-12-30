#include "userland.h"

void main (void) {
  while(1) {
    nanosleep(1000000);
    putch('!');
  }
}
