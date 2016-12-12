#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

#include "page.h"

#include <stdint.h>
#include <stddef.h>

#define NUMTHREADS 16

enum thread_state {
  TS_NONEXIST, /* Nothing here */
  TS_INACTIVE, /* Exists, but is not executing */
  TS_ACTIVE,   /* Executing, right now */
  TS_BLOCKED,  /* Blocked on IO */
};

enum fpu_state {
  THREAD_FPU_STATE_INACTIVE,  /* not used yet */
  THREAD_FPU_STATE_FORBIDDEN, /* using exits thread */
  THREAD_FPU_STATE_ACTIVE,    /* there's an FPU buf allocated */
};

typedef struct {
  enum thread_state state;
  void *rsp;        /* Kernel stack pointer, when yielded */
  void *stack_top;  /* For TSS usage */
  pagetable pt;
  enum fpu_state fpu_state;
} tcb;

extern tcb *running_tcb;

#define THREAD_FP_USE_INACTIVE (-1)
#define THREAD_FP_USE_FORBIDDEN (-2)
#define THREAD_FP_USE_DUMMY (-3)

int user_thread_create (void *text, size_t length);
void schedule_helper (void);
void thread_exit (void);
void thread_start (void);
void schedule (void);
void wake_a_thread (void);
void block_current_thread (void);
int idle_thread_create (void);

#endif
