#ifndef __SILVOS_ALLOC_H
#define __SILVOS_ALLOC_H

#include <stdint.h>

/* bsize works as follows:
 *
 * bsize==0:   1G block
 * bsize==1: 512M block
 * bsize==2: 256M block
 *   ...
 * bsize==17   8K block
 * bsize==18   4K block
 */

uint64_t memtop;

void free_block (int bsize, uint64_t index);
void *alloc_block (int bsize);
uint64_t get_index (int bsize, void *ptr);
void initialize_allocator (uint64_t usable_mem_low, uint64_t usable_mem_high);
void *allocate_phys_page (void);
void free_phys_page (void *page);

#endif
