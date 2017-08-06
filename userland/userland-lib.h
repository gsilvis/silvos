#ifndef __SILVOS_USERLAND_LIB_H
#define __SILVOS_USERLAND_LIB_H

#include <stdint.h>

#include "userland.h"

static inline uint8_t __xchg(uint8_t *l, uint8_t v) {
  __asm__ volatile ("xchg %0, %1" : "+q"(v) : "m"(*l));
  return v;
}

static inline void lock(uint8_t *l) {
  while (__xchg(l, 1)) yield();
}

static inline uint8_t unlock(uint8_t *l) {
  return __xchg(l, 0);
}

#endif
