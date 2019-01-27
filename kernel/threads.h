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
  IPC_CALLING,        /* currently waiting for a response */
  IPC_DAEMON,         /* currently waiting for a call */
};

enum sem_state {
  SEM_NOT_WAITING,
  SEM_WAITING,
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

  /* Some interrupts on Intel push a status code after %rip; for the rest, we
   * push 0, to make the stack look the same. */
  uint64_t status_code;

  /* Special registers */
  uint64_t rip;
  uint64_t cs;
  uint64_t eflags;
  uint64_t rsp;
  uint64_t ss;
};

typedef struct _tcb {
  struct list_head wait_queue;
  struct all_registers saved_registers;
  vmcb *vm_control_block;
  uint8_t thread_id;
  enum thread_state state;
  uint64_t wakeup_deadline;  /* Only valid if sleeping in hpet.c */
  enum fpu_state fpu_state;
  char (*fpu_buf)[512];
  enum ipc_state ipc_state;
  uint8_t callee;  /* Only valid if IPC_CALLING */
  enum sem_state sem_state;
  struct list_head ready_sems;  /* All semaphores we're watching that are SET */
  struct _tcb *parent;  /* Set if the parent is waiting for us to daemonize. */
  uint8_t handler_thread_id;
  uint8_t faulting;
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

/* Combination of 'fork' and 'call/respond'.  Takes in a message (similar to
 * call/respond, but with no dest addr.)  Puts the parent in a CALLing state,
 * and returns that message in the child.  (Because it was the message just
 * sent, the child can figure out that they are the child.)  When the child
 * responds, the parent will wake up again with the child's message. */
void __attribute__((noreturn)) fork_daemon (void);

/* Make a new thread in the current VM space, starting at the given %rip with
 * the given %rsp.  All other registers are zero. */
int spawn_within_vm_space (uint64_t rip, uint64_t rsp);

/* As above, but wait for the thread to daemonize before being rescheduled. */
void __attribute__((noreturn)) spawn_daemon_within_vm_space (void);

/* Set the specified thread to be the exception handler thread for the active
 * thread, and return the old value. */
int set_handler (int);

/* Get the TCB struct for the thread with the given thread ID;  returns NULL if
 * that thread does not exist. */
tcb *get_tcb (uint64_t tid);

/* Get thread ID of running thread. */
int get_tid (void);

#endif
