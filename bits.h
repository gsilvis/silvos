#ifndef __SILVOS_BITS_H
#define __SILVOS_BITS_H

#include "threads.h"

static inline void save_esp (volatile tcb* task) {
  __asm__ __volatile__ ("movl %%esp,%0" : "=r"(task->esp) : : );
}

static inline void restore_esp (volatile tcb *task) {
  __asm__ __volatile__ ("movl %0,%%esp" : : "r"(task->esp) : "memory");
}

static inline void push_registers (void) {
  __asm__("pusha");
}

static inline void pop_registers (void) {
  __asm__("popa");
}

static inline void halt (void) {
  __asm__("hlt");
}
#endif
