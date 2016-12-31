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
  TS_EXIST,    /* There's a thread */
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

int user_thread_create (void *text, size_t length);
void schedule_helper (void);
void __attribute__((noreturn)) thread_exit (void);
void thread_start (void);
void user_thread_start (void);
void user_thread_launch (void);
void schedule (void);
int idle_thread_create (void);
int clone_thread (uint64_t fork_rsp);
int finish_fork (int thread_id);
void reschedule_thread (tcb *thread);
void yield (void);


#define wait_event(wq, cond)                       \
do {                                               \
  if (list_empty(&wq) && (cond))  break;           \
  while (!cond) {                                  \
    list_push_back(&running_tcb->wait_queue, &wq); \
    schedule();                                    \
  }                                                \
} while (0)


/* TODO: instead of casting buf_head to a TCB, implement offset_of */

#define wake_up(wq)                    \
do {                                   \
  tcb *t = (tcb *)list_pop_front(&wq); \
  if (!t)  break;                      \
  list_remove(&t->wait_queue);         \
  reschedule_thread(t);                \
} while (0)


#endif
