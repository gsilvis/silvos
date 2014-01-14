#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

void enable_paging (void);
int map_page (unsigned int phys, unsigned int virt);

#endif
