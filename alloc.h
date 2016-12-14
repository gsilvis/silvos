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

void free_block (int bsize, uint64_t index);
void *alloc_block (int bsize);
void initialize_allocator (void);
void *allocate_phys_page (void);
void free_phys_page (void *page);

#endif
