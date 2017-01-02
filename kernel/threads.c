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

tcb *running_tcb = 0;

static tcb tcbs[NUMTHREADS];
static int32_t total_threads = -1;  /* The idle thread doesn't count. */
static tcb *idle_tcb = 0;

/* After calling this, you must set up the kernel stack contents, rsp, and fpu
 * state if not INACTIVE. Returns null on failure.  If you want the thread to
 * ever be scheduled, you must call 'reschedule_thread' on it. */
static tcb *create_thread (void* text, size_t length) {
  static uint8_t thread_id = 0;
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      tcbs[i].wait_queue.next = &tcbs[i].wait_queue;
      tcbs[i].wait_queue.prev = &tcbs[i].wait_queue;
      tcbs[i].thread_id = thread_id++;
      tcbs[i].pt = new_pt();
      tcbs[i].text = text;
      tcbs[i].text_length = length;
      char *kernel_stack = &((char *)allocate_phys_page())[4096];
      tcbs[i].state = TS_EXIST;
      tcbs[i].stack_top = &kernel_stack[0];
      tcbs[i].fpu_state = THREAD_FPU_STATE_INACTIVE;
      total_threads++;
      return &tcbs[i];
    }
  }
  return 0;
}

static tcb* create_thread_internal (void *text, size_t length, uint64_t entry) {
  int kernel = !text;

  tcb *new_tcb = create_thread(text, length);
  if (!new_tcb) {
    return 0;
  }

  uint64_t *kernel_stack = (uint64_t *)new_tcb->stack_top;
  /* Initialize stack */
  /* Stack frame one:  user_thread_start */
  kernel_stack[-1] = kernel ? 0x10 : 0x1B;                         /* %ss */
  kernel_stack[-2] = kernel ? (uint64_t)kernel_stack : (uint64_t)LOC_USER_STACKTOP;  /* %rsp */
  kernel_stack[-3] = 0x200;                        /* EFLAGS */
  kernel_stack[-4] = kernel ? 0x40 : 0x4B;                         /* %cs */
  kernel_stack[-5] = entry;        /* %rip */
  /* Stack frame two:  schedule */
  kernel_stack[-6] = (uint64_t)thread_start;  /* %rip */
  /* 6 callee-save registers */
  new_tcb->rsp = &kernel_stack[-12];
  return new_tcb;
}

/* Returns 0 on success, negative on failure */
int user_thread_create (void *text, size_t length) {
  if (elf64_check(text, length)) {
    return -2; /* Bad elf! */
  }
  tcb *new_tcb = create_thread_internal(text, length, elf64_get_entry(text));
  if (!new_tcb) {
    return -1;
  }

  reschedule_thread(new_tcb);
  return 0;
}

void thread_launch () {
  if (running_tcb->text) {
    elf64_load(running_tcb->text);
    map_new_page(LOC_USER_STACK, PAGE_MASK__USER | PAGE_MASK_NX);
  }
}

static void idle () {
  while (1) {
    hlt();
  }
}

int idle_thread_create () {
  idle_tcb = create_thread_internal(NULL, 0, (uint64_t)idle);
  if (!idle_tcb) {
    panic("Couldn't create idle thread!");
  }

  idle_tcb->fpu_state = THREAD_FPU_STATE_FORBIDDEN;
  return 0;
}

LIST_HEAD(schedule_queue);

/* Round-robin scheduling. */
static tcb *choose_task (void) {
  if (total_threads == 0) {
    /* All threads have exited.  Power off. */
    qemu_debug_shutdown();
  }
  tcb *result = (tcb *)list_pop_front(&schedule_queue);
  return result ? result : idle_tcb;
}

/* Begin context-switch process.  This takes in the old rsp and returns the new
 * rsp, so that we can switch stacks in pure assembly.  This function should
 * only be called in threads-asm.s */
void *start_context_switch (void *old_rsp) {
  if (running_tcb) {
    running_tcb->rsp = old_rsp;
  }
  running_tcb = choose_task();
  return running_tcb->rsp;
}

/* Finish context-switch process.  No arguments needed here.  This function
 * should only be called in threads-asm.s */
void finish_context_switch (void) {
  hpet_reset_timeout();
  fpu_switch_thread();
  set_new_rsp(running_tcb->stack_top);
  insert_pt(running_tcb->pt);
}

void thread_exit (void) {
  fpu_exit_thread();
  running_tcb->state = TS_NONEXIST;
  total_threads--;
  schedule();
  panic("Rescheduled exited thread");
}

void reschedule_thread (tcb *thread) {
  if (thread == idle_tcb) {
    return;
  }
  list_push_back(&thread->wait_queue, &schedule_queue);
}

void yield (void) {
  reschedule_thread(running_tcb);
  schedule();
}

/* 'fork_pid' holds the return value of 'clone_thread', so that it makes it
 * back to 'fork' in both the parent and the child.  We can't just return it,
 * because in the child, we return from 'schedule' instead of
 * 'fork_entry_point'.  The parent always returns immediately, so it gets the
 * real return value (the child PID).  The child returns later, and gets the
 * value that 'fork' puts in fork_pid, which is 0. */

int fork_pid = 0;

void clone_thread (uint64_t fork_rsp) {
  tcb *new_tcb = create_thread(running_tcb->text, running_tcb->text_length);
  if (!new_tcb) {
    fork_pid = -1;
    return;
  }
  new_tcb->pt = duplicate_pagetable(running_tcb->pt);
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
  reschedule_thread(new_tcb);
  fork_pid = new_tcb->thread_id;
}

int fork (void) {
  fork_entry_point(); /* This call returns twice */
  int res = fork_pid;
  fork_pid = 0;
  return res;
}
