#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))
#define NUM_SEMS 4
#define ASSERT(x) if (!(x)) { DEBUG("FAILED: FALSE: " # x); }
#define ASSERT_OK(x) if (x) { DEBUG("FAILED: NONZERO: " # x); }

semaphore_id sems[NUM_SEMS];
void waker() {
  nanosleep(500000000);
  DEBUG("WAKE 0");
  ASSERT_OK(sem_set(sems[0]));
  yield();

  /* sleeper is about to wait on 0 and 1, wake it twice */
  DEBUG("WAKE 0 again");
  ASSERT_OK(sem_set(sems[0]));
  DEBUG("WAKE 1");
  ASSERT_OK(sem_set(sems[1]));
  yield();

  /* sleeper is about to watch 2, set it */
  DEBUG("WAKE 2");
  ASSERT_OK(sem_set(sems[2]));
}

void sleeper() {
  ASSERT_OK(sem_watch(sems[0]));
  DEBUG("SLEEP 0");
  /* test simple sleep-wake */
  ASSERT(sem_wait() == sems[0]);
  DEBUG("AWAKE 0");
  ASSERT_OK(sem_watch(sems[1]));
  yield();

  /* test waking both of multiple watched sems */
  DEBUG("SLEEP 0 1");
  semaphore_id first_wait = sem_wait();
  DEBUG("AWAKE 0 1 first");
  semaphore_id second_wait = sem_wait();
  DEBUG("AWAKE 0 1 second");
  ASSERT((first_wait == sems[0] && second_wait == sems[1]) ||
         (first_wait == sems[1] && second_wait == sems[0]));
  yield();

  /* test watching a semaphore that was already set */
  DEBUG("WATCH 2");
  ASSERT_OK(sem_watch(sems[2]));
  DEBUG("SLEEP 0 1 2");
  ASSERT(sem_wait() == sems[2]);
  DEBUG("AWAKE 2");

  for (int i = 0; i < NUM_SEMS; ++i) {
    if (sem_delete(sems[i])) {
      DEBUG("DELETION FAILED");
    }
  }
}

void main() {
  for (int i = 0; i < NUM_SEMS; ++i) {
    sems[i] = sem_create();
    if (sems[i] == 0) {
      DEBUG("FAILED CREATION");
    }
  }
  int child_pid = fork();
  if (child_pid == 0) {
    waker();
    DEBUG("WAKER EXITS");
  } else {
    sleeper();
    DEBUG("SLEEPER EXITS");
  }
}
