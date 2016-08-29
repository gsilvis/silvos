#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

typedef unsigned int *pagetable;

void enable_paging (void);
void insert_pt (pagetable pt);
pagetable get_current_pt (void);
pagetable new_pt (void);
int map_page (unsigned int phys, unsigned int virt);

#endif
