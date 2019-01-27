#include "userland.h"
#include "userland-lib.h"

void main (void) {
  for (char c = 'A'; c <= 'Z'; c++) {
    debug_printf("%c", c);
    yield();
  }
}
