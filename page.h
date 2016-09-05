#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

#define PAGE_MASK_PRESENT 0x0000000000000001
#define PAGE_MASK_WRITE   0x0000000000000002
#define PAGE_MASK_PRIV    0x0000000000000004
#define PAGE_MASK_WTCACHE 0x0000000000000008
#define PAGE_MASK_NOCACHE 0x0000000000000010
#define PAGE_MASK_ACCESS  0x0000000000000020
#define PAGE_MASK_SIZE    0x0000000000000080

#define PAGE_MASK__KERNEL (PAGE_MASK_PRESENT | PAGE_MASK_WRITE)
#define PAGE_MASK__USER (PAGE_MASK_PRESENT | PAGE_MASK_WRITE | PAGE_MASK_PRIV)
#define PAGE_MASK__FAKE 0x0000000000000000


#define PAGE_4K_MASK 0x000000000FFF
#define PAGE_2M_MASK 0x0000001FFFFF
#define PAGE_1G_MASK 0x00003FFFFFFF
#define PAGE_HT_MASK 0x007FFFFFFFFF
#define PAGE_VM_MASK 0xFFFFFFFFFFFF

#define PAGE_4K_SIZE 0x000000001000
#define PAGE_2M_SIZE 0x000000200000
#define PAGE_1G_SIZE 0x000040000000
#define PAGE_HT_SIZE 0x008000000000

#define PAGE_4K_OF(mem) (0x1FF & ((mem) >> 12))
#define PAGE_2M_OF(mem) (0x1FF & ((mem) >> 21))
#define PAGE_1G_OF(mem) (0x1FF & ((mem) >> 30))
#define PAGE_HT_OF(mem) (0x1FF & ((mem) >> 39))

#define PAGE_4K_ALIGN(mem) ((mem) & (~PAGE_4K_MASK))
#define PAGE_2M_ALIGN(mem) ((mem) & (~PAGE_2M_MASK))
#define PAGE_1G_ALIGN(mem) ((mem) & (~PAGE_1G_MASK))
#define PAGE_HT_ALIGN(mem) ((mem) & (~PAGE_HT_MASK))

#define PAGE_VIRT_PT_OF(mem)   (~PAGE_HT_MASK | (((mem) & ~PAGE_2M_MASK) >> 9))
#define PAGE_VIRT_PD_OF(mem)   (~PAGE_1G_MASK | (((mem) & ~PAGE_1G_MASK) >> 18))
#define PAGE_VIRT_PDPT_OF(mem)   (~PAGE_2M_MASK | (((mem) & ~PAGE_HT_MASK) >> 27))
#define PAGE_VIRT_PML4_OF(mem)   (~PAGE_4K_MASK | (((mem) & ~PAGE_VM_MASK) >> 36))

#define PAGE_NUM_ENTRIES 512

typedef unsigned long long *pagetable;

void insert_pt (pagetable pt);
pagetable get_current_pt (void);
pagetable initial_pt (void);
pagetable new_pt (void);
int map_page (unsigned long long phys, unsigned long long virt, unsigned int mode);
int unmap_page (unsigned long long virt);
int map_new_page (unsigned long long virt, unsigned int mode);

#endif
