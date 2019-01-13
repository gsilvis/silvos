#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

#include "list.h"
#include "page.h"
#include "syscall-defs.h"

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
};

typedef struct {
  pagetable pt; /* 0 if unused */
  uint8_t refcount;
} vmcb;

/* This struct is in a very particular order: it looks exactly the same as an
 * interrupt stack just after it finishes pushing all general purpose
 * registers.  This allows a neat hack where, to return to userspace, you
 * simply make one of these structs, and move %rsp there.  The most convenient
 * place for this is in the TCB, where we store the userspace register values
 * already.
 *
 * This must stay in sync with 'push_general_purpose_reg' in isr-asm.s, and
 * with 'pop_general_purpose_reg' in thread-asm.s
 */
struct all_registers {
  /* General purpose registers */
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rbp;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rbx;
  uint64_t rax;

  /* Special registers */
  uint64_t rip;
  uint64_t cs;
  uint64_t eflags;
  uint64_t rsp;
  uint64_t ss;
};

typedef struct {
  struct list_head wait_queue;
  struct all_registers saved_registers;
  vmcb *vm_control_block;
  uint8_t thread_id;
  enum thread_state state;
  uint64_t wakeup_deadline;  /* Only valid if sleeping in hpet.c */
  enum fpu_state fpu_state;
  char (*fpu_buf)[512];
  enum ipc_state ipc_state;
} tcb;

/* Pointer to the TCB of the running userspace thread, or NULL if currently
 * idle. */
extern tcb *running_tcb;

/* Set up the (sole) kernel stack.  Called in early boot. */
uint8_t *make_kernel_stack (void);

/* Save the given registers to the TCB of the running task [if any].  This
 * should be done before (nearly) anything else in an interrupt handler. */
void save_thread_registers (struct all_registers *all_registers);

/* Create a new thread based on an ELF file pointed to by the arguments. */
int user_thread_create (void *text, size_t length);

/* Add the given thread to the end of the scheduler queue, so it gets
 * rescheduled eventually. */
void reschedule_thread (tcb *thread);

/* Choose a new task and switch to it.  This does /not/ reschedule the current
 * task; this is appropriate if you intend to block that task, and have handled
 * putting it in some other wait queue. */
void __attribute__((noreturn)) schedule (void);

/* Combination of 'reschedule_thread' and 'schedule' */
void __attribute__((noreturn)) yield (void);

/* Return to userspace.  If a userspace thread was active, returns directly to
 * it.  If the idle thread was active, checks to see if any threads have become
 * schedulable, and if so, invokes the scheduler; otherwise it continues to
 * idle. */
void __attribute__((noreturn)) return_to_current_thread (void);

/* Make the named thread the active thread, and return to userspace.  If
 * new_tcp is NULL, idles. */
void __attribute__((noreturn)) switch_thread_to (tcb *new_tcb);

/* Exit the current thread, and invoke the scheduler to choose the next thread.
 * thread_exit_fault additionally prints a warning saying that something bad
 * happened.  */
void __attribute__((noreturn)) thread_exit (void);
void __attribute__((noreturn)) thread_exit_fault (void);

/* Implements the 'fork' syscall.  Make a new thread in a new VM space, cloned
 * from the active thread's.  Sets the return value of the fork syscall to the
 * new thread ID in the parent, and to 0 in the child. */
int fork (void);

/* Make a new thread in the current VM space, starting at the given %rip with
 * the given %rsp.  All other registers are zero. */
int spawn_within_vm_space (uint64_t rip, uint64_t rsp);

/* Get the TCB struct for the thread with the given thread ID;  returns NULL if
 * that thread does not exist. */
tcb *get_tcb (uint64_t tid);

#endif
