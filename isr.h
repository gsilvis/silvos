#ifndef __SILVOS_ISR_H
#define __SILVOS_ISR_H

void dumb_isr (void);
void yield_isr (void);
void doublefault_isr (void);
void spurious_isr (void);
void isr_other (void);

void isr_00 (void);
void isr_01 (void);
void isr_02 (void);
void isr_03 (void);
void isr_04 (void);
void isr_05 (void);
void isr_06 (void);
void isr_07 (void);
void isr_08 (void);
void isr_09 (void);
void isr_0A (void);
void isr_0B (void);
void isr_0C (void);
void isr_0D (void);
void isr_0E (void);
void isr_0F (void);
void isr_10 (void);
void isr_11 (void);
void isr_12 (void);
void isr_13 (void);
void isr_14 (void);
void isr_15 (void);
void isr_16 (void);
void isr_17 (void);
void isr_18 (void);
void isr_19 (void);
void isr_1A (void);
void isr_1B (void);
void isr_1C (void);
void isr_1D (void);
void isr_1E (void);
void isr_1F (void);

void isr_20 (void);
void isr_21 (void);
void kbd_isr (void);
void timer_isr (void);

#endif
