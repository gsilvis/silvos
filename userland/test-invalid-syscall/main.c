#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void main (void) {
  DEBUG("Start");
  __syscall0(NUM_SYSCALLS*2);
  DEBUG("Very High Passed");
  __syscall0(NUM_SYSCALLS);
  DEBUG("MAX Passed");
  __syscall0(-1);
  DEBUG("-1 passed");
}
