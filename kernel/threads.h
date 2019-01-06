#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

#include "ipc.h"
#include "list.h"
#include "page.h"

#include <stdint.h>
#include <stddef.h>

#define NUMTHREADS 64
#define NUMVMSPACES 16

enum thread_state {
  TS_NONEXIST, /* Nothing here */
  TS_EXIST,    /* There's a thread */
};

enum fpu_state {
  THREAD_FPU_STATE_INACTIVE,  /* not used yet */
  THREAD_FPU_STATE_FORBIDDEN, /* using exits thread */
  THREAD_FPU_STATE_ACTIVE,    /* there's an FPU buf allocated */
};

enum ipc_state {
  IPC_NOT_RECEIVING,
  IPC_RECEIVING,      /* currently waiting for an ipc message */
  IPC_RECEIVED,       /* in the middle of receiving an ipc handoff */
};

typedef struct {
  pagetable pt; /* 0 if unused */
  uint8_t refcount;
} vmcb;

typedef struct {
  struct list_head wait_queue;
  void *rsp;        /* Kernel stack pointer, when yielded */
  vmcb *vm_control_block;
  uint8_t thread_id;
  enum thread_state state;
  void *stack_top;  /* For TSS usage */
  void *text;
  size_t text_length;
  enum fpu_state fpu_state;
  char (*fpu_buf)[512];
  enum ipc_state ipc_state;
  ipc_msg inbox;  /* Only valid if ipc_state is RECEIVED */
} tcb;

extern tcb *running_tcb;

int user_thread_create (void *text, size_t length);
int idle_thread_create (void);

void schedule (void);
void reschedule_thread (tcb *thread);
void promote (tcb *thread);
void yield (void);
int fork (void);

void thread_start (void);
void thread_start_within_old_vm_space (void);
void __attribute__((noreturn)) thread_exit (void);
void __attribute__((noreturn)) thread_exit_fault (void);

void clone_thread (uint64_t fork_rsp);
void spawn_within_vm_space (uint64_t rip, uint64_t rsp);
void com_print_backtrace (void);

tcb *get_tcb (uint64_t tid);

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
