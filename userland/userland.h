#ifndef __SILVOS_USERLAND_H
#define __SILVOS_USERLAND_H

/* Abstract Syscall Interface */

#include "../kernel/syscall-defs.h"

typedef struct {
  unsigned long long addr;
  unsigned long long r1;
  unsigned long long r2;
} ipc_msg;

typedef struct {
  ipc_msg send; /* Filled in by the user */
  ipc_msg recv; /* Filled in by the sending process */
} sendrecv_op;

static inline syscall_arg __syscall(unsigned long syscallno, syscall_arg arg1, syscall_arg arg2) {
  syscall_arg out;
  __asm__ volatile("int $0x36"
                   : "=a" (out), "+c" (arg2)
                   : "a" (syscallno), "b" (arg1)
                   : "memory");
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

static inline int spawn_thread (const void *code, void *stack) {
  return (int)__syscall2(SYSCALL_SPAWN, (syscall_arg)code, (syscall_arg)stack);
}

static inline sendrecv_status __ipc (unsigned long syscallno, sendrecv_op *op) {
  sendrecv_status status;
  ipc_msg msg = op->send;
  __asm__ volatile("int $0x36"
                   : "=a" (status), "+b" (msg.addr), "+c" (msg.r1), "+d" (msg.r2)
                   : "a" (syscallno)
                   : "memory");
  op->recv = msg;
  return status;
}

static inline sendrecv_status call (sendrecv_op *op) {
  return __ipc(SYSCALL_CALL, op);
}

static inline sendrecv_status respond (sendrecv_op *op) {
  return __ipc(SYSCALL_RESPOND, op);
}

static inline semaphore_id sem_create() {
  return __syscall0(SYSCALL_SEM_CREATE);
}

static inline int sem_delete(semaphore_id sem) {
  return __syscall1(SYSCALL_SEM_DELETE, sem);
}

static inline int sem_watch(semaphore_id sem) {
  return __syscall1(SYSCALL_SEM_WATCH, sem);
}

static inline int sem_unwatch(semaphore_id sem) {
  return __syscall1(SYSCALL_SEM_UNWATCH, sem);
}

static inline semaphore_id sem_wait() {
  return __syscall0(SYSCALL_SEM_WAIT);
}

static inline int sem_set(semaphore_id sem) {
  return __syscall1(SYSCALL_SEM_SET, sem);
}

static inline int find_proc (const char *name) {
  return __syscall1(SYSCALL_FIND_PROC, (syscall_arg)name);
}

static inline int spawn_daemon (const void *code, void *stack) {
  return (int)__syscall2(SYSCALL_SPAWN_DAEMON, (syscall_arg)code, (syscall_arg)stack);
}
#endif
