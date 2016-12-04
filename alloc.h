#ifndef __SILVOS_ALLOC_H
#define __SILVOS_ALLOC_H

void initialize_allocator (void);
void *allocate_phys_page (void);
void free_phys_page (void *page);

#endif
