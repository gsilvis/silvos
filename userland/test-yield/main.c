#include "userland.h"

void main (void) {
  char c[2];
  c[1] = 0;
  for (c[0] = 'A'; c[0]<= 'Z'; c[0]++) {
    debug(c);
    yield();
  }
}
