#include "userland.h"
#define DEBUG(str) debug(str, sizeof(str))

int depth = 0;

int test_fork() {
  int ret = fork();
  if (ret < 0) {
    DEBUG("Fork failed!");
    exit();
  } else if (ret == 0) {
    ++depth;
    char child_msg[] = "Child, depth 0";
    child_msg[sizeof(child_msg)-2] = '0' + depth;
    DEBUG(child_msg);
  } else if (ret > 0) {
    char parent_msg[] = "Parent, depth 0";
    parent_msg[sizeof(parent_msg)-2] = '0' + depth;
    DEBUG(parent_msg);
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
  DEBUG("I'm the original");
}
