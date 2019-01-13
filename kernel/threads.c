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
static pagetable idle_pt;

static uint8_t *kernel_stack = 0;

uint8_t *make_kernel_stack (void) {
  uint8_t *stack_bot = (uint8_t *)allocate_phys_page();
  kernel_stack = &stack_bot[4096];
  return kernel_stack;
}

void save_thread_registers (struct all_registers* all_registers) {
  if (running_tcb != 0) {
    running_tcb->saved_registers = *all_registers;
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
static tcb *create_thread (uint64_t rip, uint64_t rsp, vmcb *vm_space) {
  static uint8_t thread_id = 1;
  for (int i = 0; i < NUMTHREADS; i++) {
    if (tcbs[i].state == TS_NONEXIST) {
      tcbs[i].wait_queue.next = &tcbs[i].wait_queue;
      tcbs[i].wait_queue.prev = &tcbs[i].wait_queue;
      tcbs[i].thread_id = thread_id++;
      tcbs[i].vm_control_block = vm_space;
      vm_space->refcount++;
      tcbs[i].state = TS_EXIST;
      tcbs[i].fpu_state = THREAD_FPU_STATE_INACTIVE;
      tcbs[i].ipc_state = IPC_NOT_RECEIVING;
      struct all_registers scratch = {
        .rip    = rip,
        .cs     = 0x4B,
        .eflags = 0x200,  /* interrupts enabled */
        .rsp    = rsp,
        .ss     = 0x1B,
      };
      tcbs[i].saved_registers = scratch;
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
  pagetable pt = new_pt();
  tcb *new_tcb = create_thread(
      elf64_get_entry(text),
      (uint64_t)LOC_USER_STACKTOP,
      get_new_vm_space(pt));
  if (!new_tcb) {
    return -1;
  }
  /* Switch to the new address space so we can memcpy the text there. */
  pagetable old_pt = get_current_pt();
  insert_pt(pt);
  elf64_load(text, pt);
  insert_pt(old_pt);

  map_new_page(pt, LOC_USER_STACK, PAGE_MASK__USER | PAGE_MASK_NX);

  reschedule_thread(new_tcb);
  return 0;
}


static void idle () {
  while (1) {
    hlt();
  }
}

/* Move the stack to the given location, pop all registers, and iretq.  This
 * function is defined in 'isr-asm.s' */
void __attribute__((noreturn)) enter_userspace (struct all_registers *all_registers);

/* Return to userspace in the current thread (either a userspace thread or the
 * idle pseudo-thread) */
static void __attribute__((noreturn)) return_to_userspace (void) {
  if (running_tcb != 0) {
    /* Return to ring 3.  Note that enter_userspace will move %rsp to inside of
     * the TCB!  This is kind of alarming, but all the function does after that
     * is pop, so it's fine.  Notably, we are returning to ring 3, so when a
     * fault occurs, it will bring is back to the normal kernel stack location.
     */
    hpet_reset_timeout();
    enter_userspace(&running_tcb->saved_registers);
  } else {
    /* We have to be careful returning to the idle thread.  Because the idle
     * thread is in ring 0, interrupts will not take us to the top of
     * kernel_stack (like they do for interrupts while in ring 3).  So, unlike
     * in the ring 3 case, we need to place our 'struct all_registers' somewhat
     * carefully.  Let's put it at the top of the kernel stack, for consistency
     * with other interrupts.
     */
    struct all_registers scratch = {
      .rip = (uint64_t)&idle,
      .cs = 0x40,
      .eflags = 0x200,  /* interrupts enabled */
      .rsp = (uint64_t)kernel_stack,
      .ss = 0x10,
      /* I'm confused here.  I thought that, when iretq-ing from ring 0 to ring
       * 0, the CPU does not pop %ss or %rsp.   If you remove those two fields,
       * though, calc.bin breaks, even though none of the tests break.  What's
       * going on there?
       */
    };
    struct all_registers* stack_top = (struct all_registers*)kernel_stack;
    stack_top[-1] = scratch;
    enter_userspace(&stack_top[-1]);
  }
}

LIST_HEAD(schedule_queue);

/* Round-robin scheduling. */
void __attribute__((noreturn)) schedule (void) {
  int num_threads = 0;
  for (int i = 0; i < NUMTHREADS; ++i) {
    tcb* thread = &tcbs[i];
    if (thread->state == TS_NONEXIST) continue;
    if (thread->ipc_state == IPC_DAEMON) continue;
    ++num_threads;
  }
  if (num_threads == 0) {
    /* All threads have exited, or are dormant daemons.  Power off. */
    qemu_debug_shutdown();
  } else {
    switch_thread_to((tcb *)list_pop_front(&schedule_queue));
  }
}

void __attribute__((noreturn)) return_to_current_thread (void) {
  if ((running_tcb == 0) && (!list_empty(&schedule_queue))) {
    /* First of all, check if there's anything better to do.  It's possible
     * that the handler that invoked us woke up a thread, which we should
     * probably switch to. */
    schedule();
  } else {
    return_to_userspace();
  }
}

void __attribute__((noreturn)) switch_thread_to (tcb *new_tcb) {
  if (new_tcb != running_tcb) {
    fpu_switch_thread();
  }
  if (new_tcb != 0) {
    insert_pt(new_tcb->vm_control_block->pt);
  } else {
    if (idle_pt == 0) {
      idle_pt = new_pt();
    }
    insert_pt(idle_pt);
  }
  running_tcb = new_tcb;

  return_to_userspace();
}

void __attribute__((noreturn)) thread_exit_fault (void) {
  com_printf("THREAD 0x%02X FAULTED\n", running_tcb->thread_id);
  thread_exit();
}

void __attribute__((noreturn)) thread_exit (void) {
  fpu_exit_thread();
  running_tcb->vm_control_block->refcount--;
  if (running_tcb->vm_control_block->refcount == 0) {
    free_pagetable(running_tcb->vm_control_block->pt);
    running_tcb->vm_control_block->pt = 0;
  }
  running_tcb->state = TS_NONEXIST;
  schedule();
}

void reschedule_thread (tcb *thread) {
  if (thread == 0) {
    return;
  }
  list_push_back(&thread->wait_queue, &schedule_queue);
}

void __attribute__((noreturn)) yield (void) {
  reschedule_thread(running_tcb);
  schedule();
}

int fork (void) {
  vmcb *new_vmcb = get_new_vm_space(
      duplicate_pagetable(running_tcb->vm_control_block->pt));
  if (!new_vmcb) {
    return -1;
  }
  tcb *new_tcb = create_thread(
      running_tcb->saved_registers.rip,
      running_tcb->saved_registers.rsp,
      new_vmcb);
  if (!new_tcb) {
    return -1;
  }
  copy_fp_buf(new_tcb, running_tcb);
  new_tcb->saved_registers = running_tcb->saved_registers;
  new_tcb->saved_registers.rax = 0; /* Set fork return value in child. */
  reschedule_thread(new_tcb);
  return new_tcb->thread_id;
}

int spawn_within_vm_space (uint64_t rip, uint64_t rsp) {
  tcb *new_tcb = create_thread(
      rip, rsp, running_tcb->vm_control_block);
  if (!new_tcb) {
    return -1;
  }
  reschedule_thread(new_tcb);
  return new_tcb->thread_id;
}

tcb *get_tcb (uint64_t tid) {
  for (int i = 0; i < NUMTHREADS; ++i) {
    if (tcbs[i].state == TS_EXIST && tcbs[i].thread_id == tid) {
      return &tcbs[i];
    }
  }
  return 0;
}
