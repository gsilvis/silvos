#ifndef __SILVOS_ALLOC_H
#define __SILVOS_ALLOC_H

void initialize_allocator (unsigned int highmem);
void *allocate_page (void);

#endif
