#ifndef __SILVOS_FPU_H
#define __SILVOS_FPU_H

#include "threads.h"

void fpu_init (void);
void fpu_switch_thread (void);
void fpu_exit_thread (void);
void fpu_activate (void);
void copy_fp_buf (tcb *target_tcb, tcb *source_tcb);

#endif
