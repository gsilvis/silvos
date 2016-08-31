#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

#define PAGE_MASK_PRESENT 0x00000001
#define PAGE_MASK_WRITE   0x00000002
#define PAGE_MASK_PRIV    0x00000004
#define PAGE_MASK_WTCACHE 0x00000008
#define PAGE_MASK_NOCACHE 0x00000010
#define PAGE_MASK_ACCESS  0x00000020
#define PAGE_MASK_SIZE    0x00000080

#define PAGE_MASK__KERNEL (PAGE_MASK_PRESENT | PAGE_MASK_WRITE )
#define PAGE_MASK__USER (PAGE_MASK_PRESENT | PAGE_MASK_WRITE | PAGE_MASK_PRIV)
#define PAGE_MASK__FAKE 0x00000000

typedef unsigned int *pagetable;

void enable_paging (void);
void insert_pt (pagetable pt);
pagetable get_current_pt (void);
pagetable initial_pt (void);
pagetable new_pt (void);
int map_page (unsigned int phys, unsigned int virt, unsigned int mode);
int unmap_page (unsigned int virt);

#endif
