#include "userland.h"

void main (void) {
  debug("Sleeping...");
  nanosleep(10000);
  debug("Sleeping for a very short time...");
  nanosleep(5);
  debug("Sleeping for no time at all...");
  nanosleep(0);
  for (int i = 0; i < 8; i++) {
    if (fork() == 0) {
      debug("sleeping child...");
      nanosleep(10000);
      debug("child awake...");
      exit();
    }
  }
  nanosleep(1000);
  debug("Done.");
}
