#include "page.h"
#include "alloc.h"

#define PAGE_MASK_PRESENT 0x00000001
#define PAGE_MASK_WRITE   0x00000002
#define PAGE_MASK_PRIV    0x00000004
#define PAGE_MASK_WTCACHE 0x00000008
#define PAGE_MASK_NOCACHE 0x00000010
#define PAGE_MASK_ACCESS  0x00000020
#define PAGE_MASK_SIZE    0x00000080

#define PAGE_MASK_DEFAULT_REAL (PAGE_MASK_PRESENT | PAGE_MASK_WRITE | PAGE_MASK_PRIV)
#define PAGE_MASK_DEFAULT_FAKE 0x00000000

void enable_paging (void) {
  /* Put in a provisional pagetable */
  insert_pt(new_pt());
  /* Turn on paging */
  unsigned int cr0;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x80000000;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
}

void insert_pt (pagetable pt) {
  __asm__("mov %0,%%cr3" : : "r"(pt) : );
}

pagetable get_current_pt (void) {
  pagetable pt;
  __asm__("mov %%cr3,%0" : "=r"(pt) : : );
  return pt;
}

/* Return a pagetable that contains enough information to set it as your
 * pagetable. */
pagetable new_pt (void) {
  pagetable pt_upper = (pagetable)allocate_phys_page();
  pagetable pt_lower = (pagetable)allocate_phys_page();

  /* Set up page table */
  pt_upper[0] = ((unsigned int) pt_lower) | PAGE_MASK_DEFAULT_REAL;
  for (int i = 1; i < 1023; i++) {
    pt_upper[i] = PAGE_MASK_DEFAULT_FAKE;
  }
  pt_upper[1023] = ((unsigned int) pt_upper) | PAGE_MASK_DEFAULT_REAL;
  /* Set up page entry directory */
  for (int j = 0; j < 1024; j++) {
    pt_lower[j] = (j * 4096) | PAGE_MASK_DEFAULT_REAL;
  }
  return pt_upper;
}

int map_page (unsigned int phys, unsigned int virt) {
  if (phys & 0xFFF) {
    return -1;
  }
  if (virt & 0xFFF) {
    return -1;
  }

  pagetable outer = (pagetable) 0xFFFFF000;
  if (!(outer[(0xFFC00000 & virt) >> 22] & PAGE_MASK_PRESENT)) {
    outer[(0xFFC00000 & virt) >> 22] = ((unsigned int) allocate_phys_page()) | PAGE_MASK_DEFAULT_REAL;
    pagetable inner = (pagetable) ((0xFFC00000 | (virt >> 10)) & 0xFFFFF000);
    for (int i = 0; i < 1024; i++) {
      inner[i] = PAGE_MASK_DEFAULT_FAKE;
    }
  }
  pagetable inner = (pagetable) ((0xFFC00000 | (virt >> 10)) & 0xFFFFF000);
  inner[(0x003FF000 & virt) >> 12] = phys | PAGE_MASK_DEFAULT_REAL;
  return 0;
}
