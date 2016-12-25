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

struct wait_queue_head {
  struct wait_queue_head *prev, *next;
};

typedef struct {
  struct wait_queue_head wait;
  uint8_t thread_id;
  enum thread_state state;
  void *rsp;        /* Kernel stack pointer, when yielded */
  void *stack_top;  /* For TSS usage */
  pagetable pt;
  void *text;
  size_t text_length;
  enum fpu_state fpu_state;
  char (*fpu_buf)[512];
} tcb;

extern tcb *running_tcb;

#define THREAD_FP_USE_INACTIVE (-1)
#define THREAD_FP_USE_FORBIDDEN (-2)
#define THREAD_FP_USE_DUMMY (-3)

int user_thread_create (void *text, size_t length);
void schedule_helper (void);
void thread_exit (void);
void thread_start (void);
void user_thread_start (void);
void user_thread_launch (void);
void schedule (void);
int idle_thread_create (void);


#define wait_event(buf_head, cond)                        \
do {                                                      \
  if ((buf_head.next == &buf_head) && (cond))  break;     \
  running_tcb->wait.next = &buf_head;                     \
  running_tcb->wait.prev = buf_head.prev;                 \
  buf_head.prev->next = &running_tcb->wait;               \
  buf_head.prev = &running_tcb->wait;                     \
  while (!cond) {                                         \
    running_tcb->state = TS_BLOCKED;                      \
    schedule();                                           \
  }                                                       \
  running_tcb->wait.prev->next = running_tcb->wait.next;  \
  running_tcb->wait.next->prev = running_tcb->wait.prev;  \
  running_tcb->wait.prev = NULL;                          \
  running_tcb->wait.next = NULL;                          \
} while (0)


/* TODO: instead of casting buf_head to a TCB, implement offset_of */

#define wake_up(buf_head)                  \
do {                                       \
  if (buf_head.next == &buf_head)  break;  \
  tcb *t = (tcb *)buf_head.next;           \
  t->state = TS_INACTIVE;                  \
} while (0)


#endif
