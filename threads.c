#include "threads.h"
#include "util.h"
#include "bits.h"
#include "pit.h"
#include "alloc.h"
#include "gdt.h"

enum thread_state {
  TS_NONEXIST, /* Nothing here */
  TS_UNSTARTED, /* Nothing's happened yet */
  TS_ACTIVE,   /* Executing, right now */
  TS_INACTIVE, /* Exists, but is not executing */
};

typedef struct {
  enum thread_state state;
  void *esp;        /* Kernel stack pointer, when yielded */
  void *stack_top;  /* For TSS usage */
} tcb;

#define NUMTHREADS 16
tcb tcbs[NUMTHREADS];

/* Returns 0 on success, -1 on failure */
int user_thread_create (void (*task)(void)) {
  int *kernel_stack = &((int *)allocate_phys_page())[1024];
  int *user_stack = &((int *)allocate_phys_page())[1024];
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      /* Initialize tcb struct */
      tcbs[i].state = TS_UNSTARTED;
      tcbs[i].stack_top = &kernel_stack[-1];
      /* Initialize stack */
      kernel_stack[-1] = 0x23;
      kernel_stack[-2] = (int) &user_stack[-1];
      kernel_stack[-3] = 0x200;
      kernel_stack[-4] = 0x1B;
      kernel_stack[-5] = (int) task;
      /* eight registers */
      tcbs[i].esp = &kernel_stack[-13];
      return 0;
    }
  }
  return -1; /* No thread available! */
}

/* Round-robin scheduling.  If no possible task to choose, panic. */
tcb *choose_task (void) {
  static int rr = -1; /* Start with thread 0 the first time we're called. */
  for (int i = 1; i <= NUMTHREADS; i++) {
    int index = (rr+i) % NUMTHREADS;
    if (tcbs[index].state == TS_UNSTARTED || tcbs[index].state == TS_INACTIVE) {
      rr = index;
      return &tcbs[index];
    }
  }
  panic("No possible threads to schedule.");
}

void schedule (void) {
  static tcb *running_tcb = 0;
  if (running_tcb) {
    __asm__("mov %%esp,%0" : "=r"(running_tcb->esp));
  }
  running_tcb->state = TS_INACTIVE;
  running_tcb = choose_task();
  set_timeout(); /* Reset pre-emption timer */
  __asm__("mov %0,%%esp" : : "r"(running_tcb->esp)); /* Switch stacks here */
  set_new_esp(running_tcb->stack_top);
  if (running_tcb->state == TS_UNSTARTED) {
    running_tcb->state = TS_ACTIVE;
    __asm__("jmp thread_start");
  } else {
    running_tcb->state = TS_ACTIVE;
  }
}
