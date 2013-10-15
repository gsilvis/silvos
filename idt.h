#ifndef __SILVOS_IDT_H
#define __SILVOS_IDT_H

void register_isr (unsigned char num, void (*handler)(void));
void insert_idt (void);

#endif
