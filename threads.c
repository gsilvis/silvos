#include "threads.h"
#include "util.h"
#include "bits.h"

enum thread_state {
  TS_NONEXIST, /* Nothing here */
  TS_CREATED,  /* NOTUSING: No execution performed yet */
  TS_BEGUN,    /* Yielded or preempted */
};

typedef struct {
  enum thread_state state;
  void *esp;
  void (*task)(void *);
  void *userdata;
} tcb;

#define NUMTHREADS 16
tcb tcbs[NUMTHREADS];

void stack_top (tcb *t) {
  t->task(t->userdata);
  t->state = TS_NONEXIST;
  yield(); /* Does not return */
  panic("Yield returned to dead thread!");
}

/* Takes a pointer to JUST BEYOND END of the stack */
/* Currently it is UNSAFE to call this while threading is active.  Doing so
   may be race-y.  However, the obvious fix of cli() and sti() makes it unsafe
   to call OUTSIDE of threading, which is worse. */
/* Returns 0 on success, -1 on failure */
int thread_create (int *stack, void (*task)(void *), void *userdata) {
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
      return 0;
    }
  }
  return -1; /* No thread available! */
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
