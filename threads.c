#include "threads.h"

#include "memory-map.h"

#include "util.h"
#include "pit.h"
#include "alloc.h"
#include "gdt.h"
#include "page.h"
#include "fpu.h"

#include <stdint.h>
#include <stddef.h>

tcb tcbs[NUMTHREADS];

/* Returns 0 on success, -1 on failure */
int user_thread_create (void *text, size_t length) {
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      tcbs[i].pt = new_pt();
      tcbs[i].text = text;
      tcbs[i].text_length = length;
      /* Set up stacks */
      uint64_t *kernel_stack = &((uint64_t *)allocate_phys_page())[512];
      /* Initialize tcb struct */
      tcbs[i].state = TS_INACTIVE;
      tcbs[i].stack_top = &kernel_stack[0];
      /* Initialize stack */
      /* Stack frame one:  user_thread_start */
      kernel_stack[-1] = 0x1B;                         /* %ss */
      kernel_stack[-2] = (uint64_t)LOC_USER_STACKTOP;  /* %rsp */
      kernel_stack[-3] = 0x200;                        /* EFLAGS */
      kernel_stack[-4] = 0x4B;                         /* %cs */
      kernel_stack[-5] = (uint64_t)LOC_TEXT;           /* %rip */
      /* Stack frame two:  schedule */
      kernel_stack[-6] = (uint64_t)user_thread_start;  /* %rip */
      /* 6 callee-save registers */
      tcbs[i].rsp = &kernel_stack[-12];
      tcbs[i].fpu_state = THREAD_FPU_STATE_INACTIVE;
      return 0;
    }
  }
  return -1; /* No thread available! */
}

void user_thread_launch () {
  map_new_page(LOC_TEXT, PAGE_MASK__USER);
  map_new_page(LOC_USER_STACK, PAGE_MASK__USER);
  memcpy((void *)LOC_TEXT, running_tcb->text, running_tcb->text_length);
}

tcb idle_tcb;

void idle () {
  while (1) {
    hlt();
  }
}

int idle_thread_create () {
  idle_tcb.state = TS_INACTIVE;
  idle_tcb.pt = get_current_pt();
  /* Set up stack */
  uint64_t *idle_stack = &((uint64_t *)allocate_phys_page())[512];
  idle_tcb.stack_top = &idle_stack[0]; /* Not used??? */
  /* Stack frame one: thread_start */
  idle_stack[-1] = 0x10;                    /* %ss */
  idle_stack[-2] = (uint64_t) idle_stack;   /* %rsp */
  idle_stack[-3] = 0x200;                   /* EFLAGS */
  idle_stack[-4] = 0x40;                    /* %cs */
  idle_stack[-5] = (uint64_t) idle;         /* %rip */
  /* Stack frame two: schedule */
  idle_stack[-6] = (uint64_t) thread_start; /* %rip */
  /* 6 callee-save registers */
  idle_tcb.rsp = &idle_stack[-12];
  idle_tcb.fpu_state = THREAD_FPU_STATE_FORBIDDEN;
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

void *schedule_rsp;
pagetable schedule_pt;

void schedule_helper (void) {
  if (running_tcb) {
    running_tcb->rsp = schedule_rsp;
    if (running_tcb->state == TS_ACTIVE) {
      running_tcb->state = TS_INACTIVE;
    }
  }
  running_tcb = choose_task();
  set_timeout(); /* Reset pre-emption timer */
  set_new_rsp(running_tcb->stack_top);
  fpu_switch_thread();
  running_tcb->state = TS_ACTIVE;
  schedule_rsp = running_tcb->rsp;
  schedule_pt = running_tcb->pt;
}

void thread_exit (void) {
  fpu_exit_thread();
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
