#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

#include <stdint.h>
#include "page-constants.h"

typedef uint64_t *pagetable;

void insert_pt (pagetable pt);
pagetable get_current_pt (void);
pagetable initial_pt (void);
pagetable new_pt (void);
int unmap_page (uint64_t virt);
int map_new_page (uint64_t virt, uint64_t mode);

#endif
