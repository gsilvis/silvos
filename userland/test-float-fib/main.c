#include "userland.h"

#define NUM_ITERS 30
#define YIELD_ABOVE 28

#define MKFIB(name,ty,op,base, attr) \
  __attribute__ (( attr )) \
  ty name(int i) { \
    if (i == 1 || i == 0) { \
      return base; \
    } else { \
      if (i > YIELD_ABOVE) { yield(); } \
      return name(i-1) op name(i-2); \
    } \
  }

MKFIB(fibs, float, +, 1.0, target("fpmath=387"))
MKFIB(fibs_sse, float, +, 1.0, target("fpmath=sse"))

MKFIB(fibl, double, +, 1.0, target("fpmath=387"))
MKFIB(fibl_sse, double, +, 1.0, target("fpmath=sse"))

void main (void) {
  debug(fibs(NUM_ITERS) == 1346269.0 ? "Y" : "F", 1);
  yield();
  debug(fibl(NUM_ITERS) == 1346269.0 ? "Y" : "F", 1);
  yield();
  debug(fibs(NUM_ITERS) != 1346269.1 ? "Y" : "F", 1);
  yield();
  debug(fibl(NUM_ITERS) != 1346269.1 ? "Y" : "F", 1);

  yield();
  debug(fibs(NUM_ITERS) == fibs_sse(NUM_ITERS) ? "Y" : "F", 1);
  yield();
  debug(fibl(NUM_ITERS) == fibl_sse(NUM_ITERS) ? "Y" : "F", 1);
}
