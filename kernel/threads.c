#include "threads.h"

#include "memory-map.h"

#include "util.h"
#include "pit.h"
#include "alloc.h"
#include "gdt.h"
#include "page.h"
#include "fpu.h"
#include "loader.h"
#include "hpet.h"

#include <stdint.h>
#include <stddef.h>

tcb tcbs[NUMTHREADS];

/* After calling this, you must set up the kernel stack contents, rsp, and fpu
 * state if not INACTIVE. Returns null on failure. */
tcb *create_thread (void* text, size_t length) {
  static uint8_t thread_id = 1;
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      tcbs[i].thread_id = thread_id++;
      tcbs[i].pm.num_entries = 0;
      tcbs[i].pt = new_pt();
      tcbs[i].text = text;
      tcbs[i].text_length = length;
      char *kernel_stack = &((char *)allocate_phys_page())[4096];
      tcbs[i].state = TS_INACTIVE;
      tcbs[i].stack_top = &kernel_stack[0];
      tcbs[i].fpu_state = THREAD_FPU_STATE_INACTIVE;
      return &tcbs[i];
    }
  }
  return 0;
}

/* Returns 0 on success, negative on failure */
int user_thread_create (void *text, size_t length) {
  if (elf64_check(text, length)) {
    return -2; /* Bad elf! */
  }
  tcb *new_tcb = create_thread(text, length);
  if (!new_tcb) {
    return -1;
  }

  uint64_t *kernel_stack = (uint64_t *)new_tcb->stack_top;
  /* Initialize stack */
  /* Stack frame one:  user_thread_start */
  kernel_stack[-1] = 0x1B;                         /* %ss */
  kernel_stack[-2] = (uint64_t)LOC_USER_STACKTOP;  /* %rsp */
  kernel_stack[-3] = 0x200;                        /* EFLAGS */
  kernel_stack[-4] = 0x4B;                         /* %cs */
  kernel_stack[-5] = elf64_get_entry(text);        /* %rip */
  /* Stack frame two:  schedule */
  kernel_stack[-6] = (uint64_t)user_thread_start;  /* %rip */
  /* 6 callee-save registers */
  new_tcb->rsp = &kernel_stack[-12];
  return -1; /* No thread available! */
}

void user_thread_launch () {
  elf64_load(running_tcb->text);
  map_new_page(LOC_USER_STACK, PAGE_MASK__USER | PAGE_MASK_NX);
}

tcb idle_tcb;

void idle () {
  while (1) {
    hlt();
  }
}

int idle_thread_create () {
  idle_tcb.thread_id = 0;
  idle_tcb.state = TS_INACTIVE;
  idle_tcb.pm.num_entries = 0;
  idle_tcb.pt = new_pt();
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
  int thread_exists = 0;
  for (int i = 1; i <= NUMTHREADS; i++) {
    int index = (rr+i) % NUMTHREADS;
    if (tcbs[index].state == TS_INACTIVE) {
      rr = index;
      return &tcbs[index];
    } else if (tcbs[index].state != TS_NONEXIST) {
      thread_exists = 1;
    }
  }
  if (thread_exists) {
    /* Nothing to do right now; idle. */
    return &idle_tcb;
  } else {
    /* All threads have exited.  Power off. */
    qemu_debug_shutdown();
  }
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
  hpet_reset_timeout(); /* Reset pre-emption timer */
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

void thread_exit_schedule (void) {
  thread_exit();
  schedule();
  panic("Rescheduled exited thread");
}

void block_current_thread (void) {
  running_tcb->state = TS_BLOCKED;
  schedule();
}

int fork_ret = 0;

int clone_thread (uint64_t fork_rsp) {
  tcb *new_tcb = create_thread(running_tcb->text, running_tcb->text_length);
  if (!new_tcb) {
    return -1;
  }

  clone_pagemap(&new_tcb->pm, &running_tcb->pm);
  /* Clone kernel stack starting at fork_entry_point. */
  uint64_t stack_depth = (uint64_t) running_tcb->stack_top - fork_rsp;
  memcpy(((char *)new_tcb->stack_top) - stack_depth,
         ((char *)running_tcb->stack_top) - stack_depth, stack_depth);
  new_tcb->rsp = ((char*)new_tcb->stack_top) - stack_depth;
  new_tcb->fpu_state = running_tcb->fpu_state;
  if (new_tcb->fpu_state == THREAD_FPU_STATE_ACTIVE) {
    new_tcb->fpu_buf = allocate_phys_page();
    memcpy(&new_tcb->fpu_buf, &running_tcb->fpu_buf, sizeof(running_tcb->fpu_buf));
  }
  return new_tcb->thread_id;
}

int finish_fork (int thread_id) {
  if (thread_id) {
    /* Parent case: nothing to do, yet. */
    return thread_id;
  }
  apply_pagemap();
  return thread_id;
}
