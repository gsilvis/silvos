#include "userland.h"
#include "userland-lib.h"

int depth = 0;

int test_fork() {
  int ret = fork();
  if (ret < 0) {
    debug("Fork failed!");
    exit();
  } else if (ret == 0) {
    /* Temporary fix to flakiness in this test */
    nanosleep(1000000);
    ++depth;
    debug_printf("Child, depth %d", depth);
  } else if (ret > 0) {
    debug_printf("Parent, depth %d", depth);
  }
  return ret;
}

void main() {
  test_fork();
  if (depth > 0) {
    exit();
  }
  yield();
  test_fork();
  test_fork();
  yield();
  if (depth > 0) {
    exit();
  }
  yield();
  debug("I'm the original");
}
