#include "threads.h"
#include "util.h"
#include "bits.h"

#define NUMTHREADS 16
tcb tcbs[NUMTHREADS];

/* Takes a pointer to JUST ABOVE THE TOP of the stack and to the BEGINNING of the function */
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

/* Round-robin scheduling.  If no poassible task to choose, panic and halt. */
tcb *choose_task (void) {
  static int rr = -1;
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
  return 0; /* Quell the compiler */
}

/* This function cannot be inlined, because it must be ASSURED that there is
   EXACTLY ONE INSTRUCTION that switches context.  This is why I never need to
   store the instruction pointer for any thread. */
void __attribute__ ((noinline)) switch_task (volatile tcb *old_task, volatile tcb *new_task) {
  if (old_task) {
    save_esp(&old_task->esp);
  } /* If not, we were switching from not-within-the-scheduler */
  restore_esp(new_task->esp); /* !!! CAN ONLY HAVE ONE CALL TO THIS !!! */
  switch (new_task->state) {
  case TS_BEGUN:
    return;
  case TS_CREATED:
    new_task->state = TS_BEGUN;
    new_task->task(new_task->userdata);
    /* Task finished, so destroy it. */
    new_task->state = TS_NONEXIST;
    schedule(); /* Never returns */
    break;
  case TS_NONEXIST:
    panic("Waking thread in non-existent state.");
    break;
  default:
    panic("Waking thread with corrupted state.");
    break;
  }
}

void schedule (void) {
  static tcb *current_tcb = 0;
  tcb *old_tcb = current_tcb;
  current_tcb = choose_task();
  switch_task(old_tcb, current_tcb);
  return;
}

void yield (void) {
  push_registers();
  schedule();
  pop_registers();
}
