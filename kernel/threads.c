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
#include "com.h"

#include <stdint.h>
#include <stddef.h>

tcb *running_tcb = 0;

static vmcb vmcbs[NUMVMSPACES];
static tcb tcbs[NUMTHREADS];
static tcb *idle_tcb = 0;

void save_thread_registers (struct all_registers* all_registers) {
  if (running_tcb != 0) {
    running_tcb->saved_registers = all_registers;
  }
}

/* Get a new VM space, and allocate a pagetable for it. */
static vmcb *get_new_vm_space (pagetable pt) {
  for (int i = 0; i < NUMVMSPACES; i++) {
    if (vmcbs[i].pt == 0) {
      vmcbs[i].pt = pt;
      return &vmcbs[i];
    }
  }
  return 0;
}

/* After calling this, you must set up the kernel stack contents, rsp, and fpu
 * state if not INACTIVE. Returns null on failure.  If you want the thread to
 * ever be scheduled, you must call 'reschedule_thread' on it. */
static tcb *create_thread (void *text, size_t length, vmcb *vm_space) {
  static uint8_t thread_id = 0;
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      tcbs[i].wait_queue.next = &tcbs[i].wait_queue;
      tcbs[i].wait_queue.prev = &tcbs[i].wait_queue;
      tcbs[i].thread_id = thread_id++;
      tcbs[i].vm_control_block = vm_space;
      vm_space->refcount++;
      tcbs[i].text = text;
      tcbs[i].text_length = length;
      char *kernel_stack = &((char *)allocate_phys_page())[4096];
      tcbs[i].state = TS_EXIST;
      tcbs[i].stack_top = &kernel_stack[0];
      tcbs[i].fpu_state = THREAD_FPU_STATE_INACTIVE;
      tcbs[i].ipc_state = IPC_NOT_RECEIVING;
      return &tcbs[i];
    }
  }
  return 0;
}

static tcb* create_thread_internal (void *text, size_t length, uint64_t entry, vmcb* vm_space) {
  int kernel = !text;

  tcb *new_tcb = create_thread(text, length, vm_space);
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
  vmcb *new_vmcb = get_new_vm_space(new_pt());
  tcb *new_tcb = create_thread_internal(text, length, elf64_get_entry(text), new_vmcb);
  if (!new_tcb) {
    return -1;
  }

  reschedule_thread(new_tcb);
  return 0;
}

void thread_launch () {
  if (running_tcb->text) {
    elf64_load(running_tcb->text);
    map_new_page(running_tcb->vm_control_block->pt, LOC_USER_STACK, PAGE_MASK__USER | PAGE_MASK_NX);
  }
}

static void idle () {
  while (1) {
    hlt();
  }
}

int idle_thread_create () {
  vmcb *new_vmcb = get_new_vm_space(new_pt());
  idle_tcb = create_thread_internal(NULL, 0, (uint64_t)idle, new_vmcb);
  if (!idle_tcb) {
    panic("Couldn't create idle thread!");
  }

  idle_tcb->fpu_state = THREAD_FPU_STATE_FORBIDDEN;
  return 0;
}

LIST_HEAD(schedule_queue);

/* Round-robin scheduling. */
static tcb *choose_task (void) {
  int num_threads = 0;
  for (int i = 0; i < NUMTHREADS; ++i) {
    tcb* thread = &tcbs[i];
    if (thread == idle_tcb) continue;
    if (thread->state == TS_NONEXIST) continue;
    /* Threads that are in IPC_RECEIVING are considered dormant.
     * If a thread is in sendrecv() but is about to wake up with a message,
     * that's different and is indicated by IPC_RECEIVED */
    if (thread->ipc_state == IPC_RECEIVING) continue;
    ++num_threads;
  }
  if (num_threads == 0) {
    /* All threads have exited, or are dormant daemons.  Power off. */
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
  insert_pt(running_tcb->vm_control_block->pt);
}

void thread_exit_fault(void) {
  com_printf("THREAD 0x%02X FAULTED\n", running_tcb->thread_id);
  thread_exit();
}

void thread_exit (void) {
  fpu_exit_thread();
  running_tcb->vm_control_block->refcount--;
  if (running_tcb->vm_control_block->refcount == 0) {
    free_pagetable(running_tcb->vm_control_block->pt);
    running_tcb->vm_control_block->pt = 0;
  }
  running_tcb->state = TS_NONEXIST;
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

void promote (tcb *thread) {
  list_remove(&thread->wait_queue);
  list_push_front(&thread->wait_queue, &schedule_queue);
}

void clone_thread (uint64_t fork_rsp) {
  vmcb *new_vmcb = get_new_vm_space(duplicate_pagetable(running_tcb->vm_control_block->pt));
  tcb *new_tcb = create_thread(running_tcb->text, running_tcb->text_length, new_vmcb);
  if (!new_tcb) {
    running_tcb->saved_registers->rax = -1;
    return;
  }
  /* Clone kernel stack starting at fork_entry_point. */
  uint64_t stack_depth = (uint64_t) running_tcb->stack_top - fork_rsp;
  memcpy(((char *)new_tcb->stack_top) - stack_depth,
         ((char *)running_tcb->stack_top) - stack_depth, stack_depth);
  new_tcb->rsp = ((char*)new_tcb->stack_top) - stack_depth;
  copy_fp_buf(new_tcb, running_tcb);

  /* Set return value in new thread, by calculating the location of
   * 'saved_registers' */
  uint64_t saved_registers_offset = (uint64_t)running_tcb->saved_registers - (uint64_t)running_tcb->stack_top;
  uint64_t new_registers_loc = (uint64_t)new_tcb->stack_top + saved_registers_offset;
  new_tcb->saved_registers = (struct all_registers *)new_registers_loc;
  new_tcb->saved_registers->rax = 0;

  running_tcb->saved_registers->rax = new_tcb->thread_id;
  reschedule_thread(new_tcb);
}

void spawn_within_vm_space (uint64_t rip, uint64_t rsp) {
  tcb *new_tcb = create_thread(running_tcb->text, running_tcb->text_length,
                               running_tcb->vm_control_block);
  if (new_tcb == 0) {
    panic("Could not create new thread.");
  }
  uint64_t *kernel_stack = (uint64_t *)new_tcb->stack_top;
  /* Initialize stack */
  /* Stack frame one:  thread_start_within_old_vm_space */
  kernel_stack[-1] = 0x1B;  /* %ss */
  kernel_stack[-2] = rsp;   /* %rsp */
  kernel_stack[-3] = 0x200; /* EFLAGS */
  kernel_stack[-4] = 0x4B;  /* %cs */
  kernel_stack[-5] = rip;   /* %rip */
  /* Stack frame two: schedule */
  kernel_stack[-6] = (uint64_t)thread_start_within_old_vm_space;
  /* 6 callee-save registers */
  new_tcb->rsp = &kernel_stack[-12];
  reschedule_thread(new_tcb);
}

tcb *get_tcb (uint64_t tid) {
  for (int i = 0; i < NUMTHREADS; ++i) {
    if (tcbs[i].state == TS_EXIST && tcbs[i].thread_id == tid) {
      return &tcbs[i];
    }
  }
  return 0;
}
