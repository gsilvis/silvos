#include "userland.h"
#define DEBUG(str) debug(str, sizeof(str))

int depth = 0;

static const long long NEW_STACK = 0x10000000;

static int bluh = 2;

void __attribute__ ((noreturn)) my_task() {
  DEBUG("I'M ALIVE");
  if (bluh == 5) {
    DEBUG("Yeah, he did it.");
  }
  exit();
}

void main() {
  if (palloc((const void *)NEW_STACK)) {
    DEBUG("ALLOC FAIL");
  }
  spawn_thread(my_task, (void *)(NEW_STACK + 0x1000));
  bluh = 5;
  DEBUG("I didn't die, and...");
}
