#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

#include "page-constants.h"

#include <stdint.h>

typedef uint64_t *pagetable;

void insert_pt (pagetable pt);
pagetable get_current_pt (void);
pagetable initial_pt (void);
pagetable new_pt (void);
pagetable duplicate_pagetable (pagetable src);
int unmap_page (uint64_t virt);
int map_new_page (uint64_t virt, uint64_t mode);
int try_remap_cow (pagetable pt, uint64_t addr);
#endif
