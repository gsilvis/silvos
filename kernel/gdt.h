#ifndef __SILVOS_GDT_H
#define __SILVOS_GDT_H

void set_new_rsp (void *rsp);
void initialize_gdt (void);
void insert_gdt (void);
void update_iomap (void);

#endif
