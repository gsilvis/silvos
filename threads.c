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
  TS_BLOCKED,  /* Blocked on IO */
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
      map_page((unsigned int)allocate_phys_page(), TEXT, PAGE_MASK__USER);
      map_page((unsigned int)allocate_phys_page(), KERNEL_STACK, PAGE_MASK__KERNEL);
      map_page((unsigned int)allocate_phys_page(), USER_STACK, PAGE_MASK__USER);
      memcpy(text, (unsigned char *)TEXT, length);
      /* Set up stacks */
      int *kernel_stack = &((int *)KERNEL_STACK)[1024];
      int *user_stack = &((int *)USER_STACK)[1024];
      /* Initialize tcb struct */
      tcbs[i].state = TS_INACTIVE;
      tcbs[i].stack_top = &kernel_stack[0];
      /* Initialize stack */
      /* Stack frame one:  thread_start */
      kernel_stack[-1] = 0x23;                  /* %ss */
      kernel_stack[-2] = (int) &user_stack[-1]; /* %esp */
      kernel_stack[-3] = 0x200;                 /* EFLAGS */
      kernel_stack[-4] = 0x1B;                  /* %cs */
      kernel_stack[-5] = (int) TEXT;            /* %eip */
      /* Stack frame two:  schedule */
      kernel_stack[-6] = (int) thread_start;    /* %eip */
      tcbs[i].esp = &kernel_stack[-6];
      insert_pt(old_pt);
      return 0;
    }
  }
  return -1; /* No thread available! */
}

tcb idle_tcb;

void idle () {
  while (1) {
    hlt();
  }
}

int idle_thread_create () {
#define IDLE_STACK 0xC0000000
  idle_tcb.state = TS_INACTIVE;
  idle_tcb.pt = get_current_pt();
  map_page((unsigned int)allocate_phys_page(), IDLE_STACK, PAGE_MASK__USER);
  /* Set up stack */
  int *idle_stack = &((int *)IDLE_STACK)[1024];
  idle_tcb.stack_top = &idle_stack[0]; /* Not used??? */
  /* Stack frame one: thread_start */
  idle_stack[-1] = 0x200;              /* EFLAGS */
  idle_stack[-2] = 0x08;               /* %cs */
  idle_stack[-3] = (int) idle;         /* %eip */
  /* Stack frame two: schedule */
  idle_stack[-4] = (int) thread_start; /* %eip */
  idle_tcb.esp = &idle_stack[-4];
  return 0;
}

/* Round-robin scheduling. */
tcb *choose_task (void) {
  static int rr = -1; /* Start with thread 0 the first time we're called. */
  for (int i = 1; i <= NUMTHREADS; i++) {
    int index = (rr+i) % NUMTHREADS;
    if (tcbs[index].state == TS_INACTIVE) {
      rr = index;
      return &tcbs[index];
    }
  }
  /* Nothing to do; idle. */
  return &idle_tcb;
}

tcb *running_tcb = 0;

void *schedule_esp;
pagetable schedule_pt;

void schedule_helper (void) {
  if (running_tcb) {
    running_tcb->esp = schedule_esp;
    if (running_tcb->state == TS_ACTIVE) {
      running_tcb->state = TS_INACTIVE;
    }
  }
  running_tcb = choose_task();
  set_timeout(); /* Reset pre-emption timer */
  set_new_esp(running_tcb->stack_top);
  running_tcb->state = TS_ACTIVE;
  schedule_esp = running_tcb->esp;
  schedule_pt = running_tcb->pt;
}

void thread_exit (void) {
  running_tcb->state = TS_NONEXIST;
}

void wake_a_thread (void) {
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_BLOCKED) {
      tcbs[i].state = TS_INACTIVE;
      break;
    }
  }
}

void block_current_thread (void) {
  running_tcb->state = TS_BLOCKED;
  schedule();
}
