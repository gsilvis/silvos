#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

#include "list.h"
#include "page.h"
#include "pagemap.h"

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
  struct list_head wait_queue;
  uint8_t thread_id;
  enum thread_state state;
  void *rsp;        /* Kernel stack pointer, when yielded */
  void *stack_top;  /* For TSS usage */
  pagetable pt;
  void *text;
  size_t text_length;
  enum fpu_state fpu_state;
  char (*fpu_buf)[512];
  pagemap pm;
} tcb;

extern tcb *running_tcb;

#define THREAD_FP_USE_INACTIVE (-1)
#define THREAD_FP_USE_FORBIDDEN (-2)
#define THREAD_FP_USE_DUMMY (-3)

int user_thread_create (void *text, size_t length);
void schedule_helper (void);
void thread_exit (void);
void __attribute__ ((noreturn)) thread_exit_schedule (void);
void thread_start (void);
void user_thread_start (void);
void user_thread_launch (void);
void schedule (void);
int idle_thread_create (void);
int clone_thread (uint64_t fork_rsp);
int finish_fork (int thread_id);


#define wait_event(wq, cond)                      \
do {                                              \
  if (list_empty(&wq) && (cond))  break;          \
  list_push_back(&running_tcb->wait_queue, &wq);  \
  while (!cond) {                                 \
    running_tcb->state = TS_BLOCKED;              \
    schedule();                                   \
  }                                               \
  list_remove(&running_tcb->wait_queue);          \
} while (0)


/* TODO: instead of casting buf_head to a TCB, implement offset_of */

#define wake_up(wq)             \
do {                            \
  if (list_empty(&wq))  break;  \
  tcb *t = (tcb *)wq.next;      \
  t->state = TS_INACTIVE;       \
} while (0)


#endif
