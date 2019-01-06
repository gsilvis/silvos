#ifndef __SILVOS_USERLAND_H
#define __SILVOS_USERLAND_H

/* Abstract Syscall Interface */

#include "../kernel/syscall-defs.h"

static inline syscall_arg __syscall(unsigned long syscallno, syscall_arg arg1, syscall_arg arg2) {
  syscall_arg out;
  __asm__ volatile("int $0x36"
                   : "=a" (out), "+c" (arg2)
                   : "a" (syscallno), "b" (arg1)
                   : "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
  return out;
}

static inline syscall_arg __syscall0(unsigned long syscallno) {
  return __syscall(syscallno, 0, 0);
}

static inline syscall_arg __syscall1(unsigned long syscallno, syscall_arg arg1) {
  return __syscall(syscallno, arg1, 0);
}

static inline syscall_arg __syscall2(unsigned long syscallno, syscall_arg arg1, syscall_arg arg2) {
  return __syscall(syscallno, arg1, arg2);
}

/* Actual Syscalls */

static inline void yield (void) {
  __syscall0(SYSCALL_YIELD);
}

static inline void putch (char c) {
  __syscall1(SYSCALL_PUTCH, c);
}

static inline void __attribute ((noreturn)) exit (void) {
  __syscall0(SYSCALL_EXIT);
  while (1); /* SYSCALL_EXIT does not return. */
}

static inline char getch (void) {
  return (char)__syscall0(SYSCALL_GETCH);
}

static inline int read (long long sector, char *dest) {
  return (int)__syscall2(SYSCALL_READ, sector, (syscall_arg)dest);
}

static inline int write (long long sector, const char *src) {
  return (int)__syscall2(SYSCALL_WRITE, sector, (syscall_arg)src);
}

static inline int palloc (const void *virt_addr) {
  return (int)__syscall1(SYSCALL_PALLOC, (syscall_arg)virt_addr);
}

static inline int pfree (const void *virt_addr) {
  return (int)__syscall1(SYSCALL_PFREE, (syscall_arg)virt_addr);
}

static inline int debug (const char *string, int len) {
  return (int)__syscall2(SYSCALL_DEBUG, (syscall_arg)string, len);
}

static inline void nanosleep (long long nanosecs) {
  __syscall1(SYSCALL_NANOSLEEP, nanosecs);
}

static inline int fork () {
  return (int)__syscall0(SYSCALL_FORK);
}

static inline void spawn_thread (const void *code, void *stack) {
  __syscall2(SYSCALL_SPAWN, (syscall_arg)code, (syscall_arg)stack);
}

static inline sendrecv_status sendrecv (sendrecv_op* op) {
  return (int)__syscall1(SYSCALL_SENDRECV, (syscall_arg)op);
}

#endif
