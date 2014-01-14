#define PAGE_MASK_PRESENT 0x00000001
#define PAGE_MASK_WRITE   0x00000002
#define PAGE_MASK_PRIV    0x00000004
#define PAGE_MASK_WTCACHE 0x00000008
#define PAGE_MASK_NOCACHE 0x00000010
#define PAGE_MASK_ACCESS  0x00000020
#define PAGE_MASK_SIZE    0x00000040

#define PAGE_MASK_DEFAULT_REAL (PAGE_MASK_PRESENT | PAGE_MASK_WRITE | PAGE_MASK_PRIV)
#define PAGE_MASK_DEFAULT_FAKE 0x00000000

typedef unsigned int pagetable[1024] __attribute__((aligned (4096)));

pagetable pt_upper;
pagetable pt_lower;

void enable_paging (void) {
  /* Set up page table */
  pt_upper[0] = ((unsigned int) &pt_lower) | PAGE_MASK_DEFAULT_REAL;
  for (int i = 1; i < 1023; i++) {
    pt_upper[i] = PAGE_MASK_DEFAULT_FAKE;
  }
  pt_upper[1023] = ((unsigned int) &pt_upper) | PAGE_MASK_DEFAULT_REAL;
  /* Set up page entry directory */
  for (int j = 0; j < 1024; j++) {
    pt_lower[j] = (j * 4096) | PAGE_MASK_DEFAULT_REAL;
  }
  /* Turn on paging */
  __asm__("mov %0,%%cr3" : : "r"(pt_upper) : );
  unsigned int cr0;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x80000000;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
}

int map_page (unsigned int phys, unsigned int virt) {
  if (phys & 0xFFF) {
    return -1;
  }
  if (virt & 0xFFF) {
    return -1;
  }

  pagetable *outer = (pagetable *) (0xFFFFF000);
  if (!((*outer)[(0xFFC00000 & virt) >> 22] & PAGE_MASK_PRESENT)) {
    return -2;
  }
  pagetable *inner = (pagetable *) ((0xFFC00000 | (virt >> 10)) & 0xFFFFF000);
  (*inner)[(0x003FF000 & virt) >> 12] = phys | PAGE_MASK_DEFAULT_REAL;
  return 0;
}
