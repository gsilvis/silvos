#include "fpu.h"

#include "threads.h"
#include "alloc.h"
#include "page.h"
#include "util.h"

struct {
  char data[512];
} *fpu_state;

int active_fp_buf = THREAD_FP_USE_DUMMY;

void fpu_init (void) {
  /* Set up buffers to save FPU information in */
  fpu_state = (void *) 0x00150000;
  int num_fpbuf_pages = (NUMFPBUFS - 1) / 8 + 1;
  for (int i = 0; i < num_fpbuf_pages; i++) {
    map_page((unsigned long long)allocate_phys_page(),
             0x00150000 + 0x1000 * i,
             PAGE_MASK__KERNEL);
  }
  memset(fpu_state, 0, 512 * NUMFPBUFS);
  /* Enable FPU */
  unsigned long long cr0, cr4;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x0000002A;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
  __asm__("mov %%cr4,%0" : "=r"(cr4) : : );
  cr4 |= 0x00000600;
  __asm__("mov %0,%%cr4" : : "r"(cr4) : );
}

void switch_fp_buf (int new_fp_buf) {
  if (new_fp_buf != active_fp_buf) {
    if (active_fp_buf >= 0) {
      __asm__("fxsave64 %0" : : "m"(fpu_state[active_fp_buf]) : );
    }
    active_fp_buf = new_fp_buf;
    __asm__("fxrstor64 %0" : : "m"(fpu_state[active_fp_buf]) : );
  }
}

void disable_fpu (void) {
  unsigned long long cr0;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x00000008;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
}

void enable_fpu (void) {
  __asm__("clts");
}

void fpu_switch_thread (void) {
  if (running_tcb->fp_buf >= 0) {
    enable_fpu();
    switch_fp_buf(running_tcb->fp_buf);
  } else {
    disable_fpu();
  }
}

void fpu_activate (void) {
  if (running_tcb->fp_buf == THREAD_FP_USE_FORBIDDEN) {
    thread_exit();
  }
  /* fp_buf != THREAD_FP_USE_FORBIDDEN */
  if (running_tcb->fp_buf == THREAD_FP_USE_INACTIVE) {
    static int next_fp_buf = 0;
    running_tcb->fp_buf = next_fp_buf++;
  }
  enable_fpu();
  switch_fp_buf(running_tcb->fp_buf);
}
