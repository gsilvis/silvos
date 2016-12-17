#ifndef __SILVOS_FPU_H
#define __SILVOS_FPU_H

void fpu_init (void);
void fpu_switch_thread (void);
void fpu_exit_thread (void);
void fpu_activate (void);

#endif
