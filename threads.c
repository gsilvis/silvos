#include "threads.h"
#include "util.h"
#include "bits.h"

#define NUMTHREADS 16
tcb tcbs[NUMTHREADS];

void stack_top (tcb *t) {
  t->task(t->userdata);
  t->state = TS_NONEXIST;
  yield(); /* Does not return */
  panic("Yield returned to dead thread!");
}

/* Takes a pointer to JUST BEYOND END of the stack */
tcb *thread_create (int *stack, void (*task)(void *), void *userdata) {
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      /* Initialize tcb struct */
      tcbs[i].state = TS_CREATED;
      tcbs[i].task = task;
      tcbs[i].userdata = userdata;
      /* Initialize stack */
      stack[-1] = (int) &tcbs[i]; /* Argument to stack_top */
      /* Return address from stack_top is irrelevant */
      stack[-3] = 0x200; /* EFLAGS:  allow interrupts, but that's it */
      stack[-4] = 0x08; /* Segment */
      stack[-5] = (int) &stack_top; /* Return address from ISR */
      /* Eight 4-byte registers for POPA */
      tcbs[i].esp = stack - 13;
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


/* Takes in and returns an esp pointer */
void *schedule (void *esp) {
  static tcb *running_tcb = 0;
  if (running_tcb) {
    running_tcb->esp = esp;
  }
  running_tcb = choose_task();
  return running_tcb->esp;
}

void yield (void) {
  __asm__("int $0x36");
}
