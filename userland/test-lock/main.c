#include "userland.h"

#include "userland-lib.h"

static const long long NEW_STACK = 0x10000000;
static const long long NEW_STACK2 = 0x10004000;

static volatile int bluh = 0;
static volatile int done = 0;
static uint8_t mylock = 0;

void __attribute__ ((noreturn)) my_task() {
  debug("START THREAD");
  for (int i = 0; i < 100; i++) {
    lock(&mylock);
    bluh++;
    unlock(&mylock);
    nanosleep(1);
  }
  lock(&mylock);
  done++;
  unlock(&mylock);
  debug("THREAD DONE");
  exit();
}

void main() {
  if (palloc((const void *)NEW_STACK)) {
    debug("ALLOC FAIL");
  }
  if (palloc((const void *)NEW_STACK2)) {
    debug("ALLOC FAIL");
  }

  spawn_thread(my_task, (void *)(NEW_STACK + 0x1000));
  spawn_thread(my_task, (void *)(NEW_STACK2 + 0x1000));

  while (1) {
    lock(&mylock);
    if (done != 2) {
      unlock(&mylock);
      yield();
      continue;
    }
    if (bluh == 200) {
      debug("LOOKS GOOD");
    } else {
      debug("FAIL");
    }

    unlock(&mylock);
    return;
  }
}
