#ifndef __SILVOS_KBD_H
#define __SILVOS_KBD_H

void init_kbd (void);
void read_key (void);
void __attribute__((noreturn)) getch (void);

#endif
