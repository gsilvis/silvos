#ifndef __SILVOS_GDT_H
#define __SILVOS_GDT_H

void set_new_esp (void *esp);
void initialize_gdt (void);
void insert_gdt (void);

#endif
