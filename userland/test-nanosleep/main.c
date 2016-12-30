#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void main (void) {
  DEBUG("Sleeping...");
  nanosleep(10000);
  DEBUG("Sleeping for a very short time...");
  nanosleep(5);
  DEBUG("Sleeping for no time at all...");
  nanosleep(0);
  DEBUG("Done.");
}
