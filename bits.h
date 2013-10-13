#ifndef __SILVOS_BITS_H
#define __SILVOS_BITS_H

static inline void save_esp (void * volatile *a) {
  __asm__ __volatile__ ("movl %%esp,%0" : "=r"(*a) : : );
}

static inline void restore_esp (const volatile void *a) {
  __asm__ __volatile__ ("movl %0,%%esp" : : "r"(a) : "memory");
}

static inline void push_registers (void) {
  __asm__("pusha");
}

static inline void pop_registers (void) {
  __asm__("popa");
}

static inline __attribute__ ((noreturn)) void halt (void) {
  while (1) {
    __asm__("hlt");
  }
}

#endif
