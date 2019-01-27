#include "userland.h"

#define NUM_SEMS 4
#define ASSERT(x) if (!(x)) { debug("FAILED: FALSE: " # x); }
#define ASSERT_OK(x) if (x) { debug("FAILED: NONZERO: " # x); }

semaphore_id sems[NUM_SEMS];
void waker() {
  nanosleep(500000000);
  debug("WAKE 0");
  ASSERT_OK(sem_set(sems[0]));
  yield();

  /* sleeper is about to wait on 0 and 1, wake it twice */
  debug("WAKE 0 again");
  ASSERT_OK(sem_set(sems[0]));
  debug("WAKE 1");
  ASSERT_OK(sem_set(sems[1]));
  yield();

  /* sleeper is about to watch 2, set it */
  debug("WAKE 2");
  ASSERT_OK(sem_set(sems[2]));
}

void sleeper() {
  ASSERT_OK(sem_watch(sems[0]));
  debug("SLEEP 0");
  /* test simple sleep-wake */
  ASSERT(sem_wait() == sems[0]);
  debug("AWAKE 0");
  ASSERT_OK(sem_watch(sems[1]));
  yield();

  /* test waking both of multiple watched sems */
  debug("SLEEP 0 1");
  semaphore_id first_wait = sem_wait();
  debug("AWAKE 0 1 first");
  semaphore_id second_wait = sem_wait();
  debug("AWAKE 0 1 second");
  ASSERT((first_wait == sems[0] && second_wait == sems[1]) ||
         (first_wait == sems[1] && second_wait == sems[0]));
  yield();

  /* test watching a semaphore that was already set */
  debug("WATCH 2");
  ASSERT_OK(sem_watch(sems[2]));
  debug("SLEEP 0 1 2");
  ASSERT(sem_wait() == sems[2]);
  debug("AWAKE 2");

  for (int i = 0; i < NUM_SEMS; ++i) {
    if (sem_delete(sems[i])) {
      debug("DELETION FAILED");
    }
  }
}

void main() {
  for (int i = 0; i < NUM_SEMS; ++i) {
    sems[i] = sem_create();
    if (sems[i] == 0) {
      debug("FAILED CREATION");
    }
  }
  int child_pid = fork();
  if (child_pid == 0) {
    waker();
    debug("WAKER EXITS");
  } else {
    sleeper();
    debug("SLEEPER EXITS");
  }
}
