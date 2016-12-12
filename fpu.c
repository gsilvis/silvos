#include "fpu.h"

#include "memory-map.h"

#include "threads.h"
#include "alloc.h"
#include "page.h"
#include "util.h"

tcb *previous_tcb = NULL; /* always the same as which one is mapped in LOC_FP_BUF */

struct {
  uint8_t data[512];
} *const fp_buf = (void *)LOC_FP_BUF;

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
    __asm__("fxsave64 %0" : : "m"(fp_buf) : );
    unmap_page(LOC_FP_BUF);
  }
  previous_tcb = new_tcb;
  remap_page(LOC_THREAD_FP, LOC_FP_BUF, PAGE_MASK__KERNEL);
  __asm__("fxrstor64 %0" : : "m"(fp_buf) : );
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
    unmap_page(LOC_FP_BUF);
  }
  if (running_tcb->fpu_state == THREAD_FPU_STATE_ACTIVE) {
    running_tcb->fpu_state = THREAD_FPU_STATE_INACTIVE;
    free_phys_page((void *)virt_to_phys(LOC_THREAD_FP));
  }
}

void fpu_activate (void) {
  if (running_tcb->fpu_state == THREAD_FPU_STATE_FORBIDDEN) {
    thread_exit();
  }
  /* fp_buf != THREAD_FP_USE_FORBIDDEN */
  if (running_tcb->fpu_state == THREAD_FPU_STATE_INACTIVE) {
    map_new_page(LOC_THREAD_FP, PAGE_MASK__KERNEL);
    running_tcb->fpu_state = THREAD_FPU_STATE_ACTIVE;
  }
  enable_fpu();
  switch_fp_buf(running_tcb);
}
