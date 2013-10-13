#include "threads.h"
#include "util.h"
#include "bits.h"

#define NUMTHREADS 16
tcb tcbs[NUMTHREADS];

/* Takes a pointer to TOP USABLE WORD of the stack */
tcb *thread_create (void *stack, void (*task)(void *), void *userdata) {
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      /* Found an avilable thread */
      tcbs[i].state = TS_CREATED;
      tcbs[i].esp = stack;
      tcbs[i].task = task;
      tcbs[i].userdata = userdata;
      return &tcbs[i];
    }
  }
  return 0; /* No thread available! */
}

/* Round-robin scheduling.  If no poassible task to choose, panic. */
tcb *choose_task (void) {
  static int rr = -1; /* Start with thread 0 the first time we're called. */
  for (int i = 1; i <= NUMTHREADS; i++) {
    int index = (rr+i) % NUMTHREADS;
    if (tcbs[index].state == TS_CREATED || tcbs[index].state == TS_BEGUN) {
      rr = index;
      return &tcbs[index];
    } else if (tcbs[index].state != TS_NONEXIST) {
      panic("Invalid thread state.");
    }
  }
  panic("No possible threads to schedule.");
}

/* This function cannot be inlined or cloned, because it must be ASSURED that
 * there is EXACTLY ONE INSTRUCTION that switches context.  This is why I
 * never need to store the instruction pointer for any thread.  Furthermore,
 * %esp must be saved and restored in the same function, so again in only one
 * place.
 *
 * There will be a stack frame for this function at the bottom and top of
 * every thread stack (except the bottom of the currently active stack). */
void __attribute__ ((noinline, noclone)) switch_task (volatile tcb *old_task, volatile tcb *new_task) {
  if (old_task) {
    save_esp(&old_task->esp);
  } /* If not, we were switching from something that wasn't a thread. */
  restore_esp(new_task->esp);
  switch (new_task->state) {
  case TS_BEGUN: /* Bottom of stack */
    return;
  case TS_CREATED: /* Top of stack */
    new_task->state = TS_BEGUN;
    new_task->task(new_task->userdata);
    exit();
  case TS_NONEXIST:
    panic("Waking uncreated or exited thread.");
  default:
    panic("Waking thread in corrupted state.");
  }
}

tcb *current_tcb = 0;

void schedule (void) {
  tcb *old_tcb = current_tcb;
  current_tcb = choose_task();
  switch_task(old_tcb, current_tcb);
}

void __attribute__ ((noreturn)) exit (void) {
  current_tcb->state = TS_NONEXIST;
  schedule(); /* Does not return */
  panic("Scheduler somehow returned into exited thread.");
}

void yield (void) {
  push_registers();
  schedule();
  pop_registers();
}
