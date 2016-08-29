#include "userland.h"

void main (void) {
  while (1) {
    putch('a');
    yield();
  }
}
