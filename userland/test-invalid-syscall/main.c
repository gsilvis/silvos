#include "userland.h"

void main (void) {
  debug("Start");
  __syscall0(NUM_SYSCALLS*2);
  debug("Very High Passed");
  __syscall0(NUM_SYSCALLS);
  debug("MAX Passed");
  __syscall0(-1);
  debug("-1 passed");
}
