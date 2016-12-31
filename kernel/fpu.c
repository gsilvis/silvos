#include "fpu.h"

#include "memory-map.h"

#include "threads.h"
#include "alloc.h"
#include "page.h"
#include "util.h"

tcb *previous_tcb = NULL;

void fpu_init (void) {
  /* Enable FPU */
  uint64_t cr0, cr4;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x0000002A;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
  __asm__("mov %%cr4,%0" : "=r"(cr4) : : );
  cr4 |= 0x00000600;
  __asm__("mov %0,%%cr4" : : "r"(cr4) : );
}

void switch_fp_buf (tcb *new_tcb) {
  if (new_tcb == previous_tcb) {
    return;
  }
  if (previous_tcb) {
    __asm__("fxsave64 %0" : : "m"(*previous_tcb->fpu_buf) : );
  }
   blab();
  __asm__("fxrstor64 %0" : : "m"(*new_tcb->fpu_buf) : );
  previous_tcb = new_tcb;
}

void disable_fpu (void) {
  uint64_t cr0;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x00000008;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
}

void enable_fpu (void) {
  __asm__("clts");
}

void fpu_switch_thread (void) {
  /* For now, do nothing, and switch lazily. */
  disable_fpu();
}

void fpu_exit_thread (void) {
  if (running_tcb == previous_tcb) {
    previous_tcb = 0;
  }
  if (running_tcb->fpu_state == THREAD_FPU_STATE_ACTIVE) {
    running_tcb->fpu_state = THREAD_FPU_STATE_INACTIVE;
    free_phys_page((void *)running_tcb->fpu_buf);
  }
}

void fpu_activate (void) {
  if (running_tcb->fpu_state == THREAD_FPU_STATE_FORBIDDEN) {
    thread_exit();
  }
  /* fp_buf != THREAD_FP_USE_FORBIDDEN */
  if (running_tcb->fpu_state == THREAD_FPU_STATE_INACTIVE) {
    /* TODO: this is more memory than is actually needed. */
    running_tcb->fpu_buf = allocate_phys_page();
    running_tcb->fpu_state = THREAD_FPU_STATE_ACTIVE;
  }
  enable_fpu();
  switch_fp_buf(running_tcb);
}
