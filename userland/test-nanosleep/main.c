#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void main (void) {
  DEBUG("Sleeping...");
  nanosleep(10000);
  DEBUG("Sleeping for a very short time...");
  nanosleep(5);
  DEBUG("Sleeping for no time at all...");
  nanosleep(0);
  for (int i = 0; i < 8; i++) {
    if (fork() == 0) {
      DEBUG("sleeping child...");
      nanosleep(10000);
      DEBUG("child awake...");
      exit();
    }
  }
  nanosleep(1000);
  DEBUG("Done.");
}
