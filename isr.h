#ifndef __SILVOS_ISR_H
#define __SILVOS_ISR_H

void dumb_isr (void);
void yield_isr (void);
void doublefault_isr (void);
void spurious_isr (void);
void isr_other (void);
void isr_20 (void);
void isr_21 (void);
void kbd_isr (void);
void timer_isr (void);

#endif
