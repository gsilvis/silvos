#ifndef __SILVOS_ISR_H
#define __SILVOS_ISR_H

void syscall_isr (void);

void fault_isr (void);
void nm_isr (void);
void df_isr (void);
void pf_isr (void);

void kbd_isr (void);
void timer_isr (void);
void rtc_isr (void);
void ide_isr (void);

#endif
