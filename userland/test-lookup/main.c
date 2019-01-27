#include "userland.h"

void main (void) {
  fork();
  if (find_proc("test-lookup.bin") != 1) {
    debug("FOUND WRONG THREAD");
  }
}
