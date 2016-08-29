#include "threads.h"
#include "util.h"
#include "bits.h"
#include "pit.h"
#include "alloc.h"
#include "gdt.h"
#include "page.h"

enum thread_state {
  TS_NONEXIST, /* Nothing here */
  TS_INACTIVE, /* Exists, but is not executing */
  TS_ACTIVE,   /* Executing, right now */
};

typedef struct {
  enum thread_state state;
  void *esp;        /* Kernel stack pointer, when yielded */
  void *stack_top;  /* For TSS usage */
  pagetable pt;
} tcb;

#define NUMTHREADS 16
tcb tcbs[NUMTHREADS];

/* Returns 0 on success, -1 on failure */
int user_thread_create (unsigned char *text, unsigned int length) {
#define TEXT 0x40000000
#define KERNEL_STACK 0x40001000
#define USER_STACK 0x40002000
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      /* Make new page table */
      pagetable old_pt = get_current_pt();
      tcbs[i].pt = new_pt();
      insert_pt(tcbs[i].pt);
      map_page((unsigned int)allocate_phys_page(), TEXT);
      map_page((unsigned int)allocate_phys_page(), KERNEL_STACK);
      map_page((unsigned int)allocate_phys_page(), USER_STACK);
      memcpy(text, (unsigned char *)TEXT, length);
      /* Set up stacks */
      int *kernel_stack = &((int *)KERNEL_STACK)[1024];
      int *user_stack = &((int *)USER_STACK)[1024];
      /* Initialize tcb struct */
      tcbs[i].state = TS_INACTIVE;
      tcbs[i].stack_top = &kernel_stack[-1];
      /* Initialize stack */
      kernel_stack[-1] = 0x23;
      kernel_stack[-2] = (int) &user_stack[-4];
      kernel_stack[-3] = 0x200;
      kernel_stack[-4] = 0x1B;
      kernel_stack[-5] = (int) TEXT;
      user_stack[-1] = 0x9090;
      /* eight registers */
      tcbs[i].esp = &kernel_stack[-13];
      insert_pt(old_pt);
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
    if (tcbs[index].state == TS_INACTIVE) {
      rr = index;
      return &tcbs[index];
    }
  }
  panic("No possible threads to schedule.");
}

tcb *running_tcb = 0;

void *schedule_esp;
pagetable schedule_pt;

void schedule (void) {
  if (running_tcb) {
    running_tcb->esp = schedule_esp;
  }
  if (running_tcb->state == TS_ACTIVE) {
    running_tcb->state = TS_INACTIVE;
  }
  running_tcb = choose_task();
  set_timeout(); /* Reset pre-emption timer */
  set_new_esp(running_tcb->stack_top);
  running_tcb->state = TS_ACTIVE;
  schedule_esp = running_tcb->esp;
  schedule_pt = running_tcb->pt;
}

void thread_exit(void) {
  running_tcb->state = TS_NONEXIST;
}
