#include "userland.h"

#define NUM_ITERS 20
#define YIELD_ABOVE 15
#define EXPECTED 10946.00

#define MKFIB(name,ty,op,base, attr) \
  __attribute__ (( attr )) \
  void name ## _inner (int i, ty *out) { \
    if (i == 1 || i == 0) { \
      *out = base; \
    } else { \
      if (i > YIELD_ABOVE) { yield(); } \
      ty a, b; \
      name ## _inner (i-1, &a); \
      name ## _inner (i-2, &b); \
      *out = a op b; \
    } \
  } \
  ty name (int i) { \
    ty res; \
    name ## _inner (i, &res); \
    return res; \
  }

MKFIB(fibs, float, +, 1.0, )
MKFIB(fibs_387, float, +, 1.0, target("fpmath=387,no-sse,no-sse2"))
MKFIB(fibs_sse, float, +, 1.0, target("fpmath=sse"))

MKFIB(fibl, double, +, 1.0, )
MKFIB(fibl_387, double, +, 1.0, target("fpmath=387,no-sse,no-sse2"))
MKFIB(fibl_sse, double, +, 1.0, target("fpmath=sse"))

void main (void) {
  float x = EXPECTED;
  double y = EXPECTED;

  unsigned long long pid = 0;

  /* Test presisting an FPU value over a context switch */
  __asm__ volatile (
   "flds (%1)\n"
   "movl $0x0, (%1)\n"
   "int $0x36\n"
   "fsts (%1)\n" : "=a"(pid) : "b"(&x), "a"(SYSCALL_FORK): "memory");
  debug(x == EXPECTED ? "Y" : "F");
  if (!pid) { exit(); }
  yield();

  /* Same test, but with a double */
  __asm__ volatile (
   "fldl (%1)\n"
   "movq $0x0, (%1)\n"
   "int $0x36\n"
   "fstl (%1)\n" : "=a"(pid) : "b"(&y), "a"(SYSCALL_FORK): "memory");
  debug(y == EXPECTED ? "Y" : "F");
  if (!pid) { exit(); }
  yield();

  /* Test presisting an SSE value over a context switch */
  __asm__ volatile (
   "movss (%1), %%xmm0\n"
   "movl $0x0, (%1)\n"
   "int $0x36\n"
   "movss %%xmm0, (%1)\n" : "=a"(pid) : "b"(&x), "a"(SYSCALL_FORK): "memory");
  debug(x == EXPECTED ? "Y" : "F");
  if (!pid) { exit(); }
  yield();

  /* Same test, but with a double */
  __asm__ volatile (
   "movsd (%1), %%xmm0\n"
   "movq $0x0, (%1)\n"
   "int $0x36\n"
   "movsd %%xmm0, (%1)\n" : "=a"(pid) : "b"(&y), "a"(SYSCALL_FORK): "memory");
  debug(y == EXPECTED ? "Y" : "F");
  if (!pid) { exit(); }
  yield();

  /* Test doing some simple math */
  debug(fibs(NUM_ITERS) == EXPECTED ? "Y" : "F");
  yield();
  debug(fibl(NUM_ITERS) == EXPECTED ? "Y" : "F");
  yield();
  debug(fibs(NUM_ITERS) != EXPECTED / 2 ? "Y" : "F");
  yield();
  debug(fibl(NUM_ITERS) != EXPECTED / 2 ? "Y" : "F");
  yield();
  debug(fibs(NUM_ITERS) == fibs_387(NUM_ITERS) ? "Y" : "F");
  yield();
  debug(fibl(NUM_ITERS) == fibl_387(NUM_ITERS) ? "Y" : "F");
  yield();
  debug(fibs(NUM_ITERS) == fibs_sse(NUM_ITERS) ? "Y" : "F");
  yield();
  debug(fibl(NUM_ITERS) == fibl_sse(NUM_ITERS) ? "Y" : "F");
}
