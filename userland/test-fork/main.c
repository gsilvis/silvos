#include "userland.h"

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
    char child_msg[] = "Child, depth 0";
    child_msg[sizeof(child_msg)-2] = '0' + depth;
    debug(child_msg);
  } else if (ret > 0) {
    char parent_msg[] = "Parent, depth 0";
    parent_msg[sizeof(parent_msg)-2] = '0' + depth;
    debug(parent_msg);
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
