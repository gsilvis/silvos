#include "userland.h"

void main (void) {
  for (char c = 'A'; c <= 'Z'; c++) {
    debug(&c, 1);
    yield();
  }
}
