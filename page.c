#include "page.h"

#include "alloc.h"

pagetable kernel_page;

void enable_paging (void) {
  /* Put in a provisional pagetable */
  insert_pt(initial_pt());
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

extern int _end;

/* This should be called before paging is set up */
pagetable initial_pt (void) {
  /* Set up the shared kernel page. */
  kernel_page = (pagetable)allocate_phys_page();

  unsigned int first_blank_page = (unsigned int)&_end;
  if (first_blank_page & 0xFFF) {
    first_blank_page &= 0xFFFFF000;
    first_blank_page += 0x1000;
  }
  first_blank_page = first_blank_page >> 12;

  for (unsigned int j = 0x000; j < 0x100; j++) {
    kernel_page[j] = PAGE_MASK__FAKE;
  }
  for (unsigned int j = 0x100; j < first_blank_page + 25; j++) {
    kernel_page[j] = (j * 4096) | PAGE_MASK__KERNEL;
  }
  for (unsigned int j = first_blank_page + 25; j < 0x400; j++) {
    kernel_page[j] = PAGE_MASK__FAKE;
  }
  kernel_page[0xB8] = 0xB8000 | PAGE_MASK__KERNEL; /* VGA */
  /* Set up root of page table. */
  pagetable pt_upper = (pagetable)allocate_phys_page();

  pt_upper[0] = ((unsigned int) kernel_page) | PAGE_MASK__USER;
  for (int i = 1; i < 1023; i++) {
    pt_upper[i] = PAGE_MASK__FAKE;
  }
  pt_upper[1023] = ((unsigned int) pt_upper) | PAGE_MASK__KERNEL;

  return pt_upper;
}

/* Return a pagetable that contains enough information to set it as your
 * pagetable. */
pagetable new_pt (void) {
  pagetable pt_phys = (pagetable)allocate_phys_page();

  pagetable pt_virt = (pagetable)0x00001000;

  map_page((unsigned int)pt_phys, (unsigned int)pt_virt, PAGE_MASK__KERNEL);

  pt_virt[0] = ((unsigned int) kernel_page) | PAGE_MASK__KERNEL;
  for (int i = 1; i < 1023; i++) {
    pt_virt[i] = PAGE_MASK__FAKE;
  }
  pt_virt[1023] = ((unsigned int) pt_phys) | PAGE_MASK__KERNEL;

  unmap_page((unsigned int)pt_virt);

  return pt_phys;
}

int map_page (unsigned int phys, unsigned int virt, unsigned int mode) {
  if (phys & 0xFFF) {
    return -1;
  }
  if (virt & 0xFFF) {
    return -1;
  }

  pagetable outer = (pagetable) 0xFFFFF000;
  if (!(outer[(0xFFC00000 & virt) >> 22] & PAGE_MASK_PRESENT)) {
    outer[(0xFFC00000 & virt) >> 22] = ((unsigned int) allocate_phys_page()) | PAGE_MASK__USER;
    pagetable inner = (pagetable) ((0xFFC00000 | (virt >> 10)) & 0xFFFFF000);
    for (int i = 0; i < 1024; i++) {
      inner[i] = PAGE_MASK__FAKE;
    }
  }
  pagetable inner = (pagetable) ((0xFFC00000 | (virt >> 10)) & 0xFFFFF000);
  inner[(0x003FF000 & virt) >> 12] = phys | mode;
  return 0;
}

int unmap_page (unsigned int virt) {
  if (virt & 0xFFF) {
    return -1;
  }

  pagetable outer = (pagetable) 0xFFFFF000;
  if (!(outer[(0xFFC00000 & virt) >> 22] & PAGE_MASK_PRESENT)) {
    return 0;
  }
  pagetable inner = (pagetable) ((0xFFC00000 | (virt >> 10)) & 0xFFFFF000);
  inner[(0x003FF000 & virt) >> 12] = PAGE_MASK__FAKE;
  return 0;
}
