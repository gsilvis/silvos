#ifndef __SILVOS_ISR_H
#define __SILVOS_ISR_H

void yield_isr (void);
void putch_isr (void);
void exit_isr (void);
void getch_isr (void);
void read_isr (void);
void write_isr (void);

void fault_isr (void);
void nm_isr (void);
void df_isr (void);
void pf_isr (void);

void kbd_isr (void);
void timer_isr (void);

#endif
