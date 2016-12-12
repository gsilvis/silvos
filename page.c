#include "page.h"

#include "memory-map.h"

#include "alloc.h"
#include "util.h"

#include <stdint.h>

pagetable kernel_pt;

void insert_pt (pagetable pt) {
  __asm__("mov %0,%%cr3" : : "r"(pt) : );
}

pagetable get_current_pt (void) {
  pagetable pt;
  __asm__("mov %%cr3,%0" : "=r"(pt) : : );
  return pt;
}

extern int _end;

pagetable initial_pt (void) {
  /* Set up the shared kernel pt */
  kernel_pt = (pagetable)allocate_phys_page();
  memset(kernel_pt, 0x00, PAGE_4K_SIZE);

  uint64_t first_blank_page = (uint64_t)&_end;
  if (first_blank_page & PAGE_4K_MASK) {
    first_blank_page = PAGE_4K_ALIGN(first_blank_page) + PAGE_4K_SIZE;
  }
  first_blank_page = PAGE_4K_OF(first_blank_page);

  for (unsigned int j = 0x00; j < 0x80; j++) {
    /* Don't map low memory */
    kernel_pt[j] = PAGE_MASK__FAKE;
  }
  for (unsigned int j = 0x80; j < first_blank_page; j++) {
    kernel_pt[j] = (j * 4096) | PAGE_MASK__KERNEL;
  }
  for (unsigned int j = first_blank_page; j < PAGE_NUM_ENTRIES; j++) {
    kernel_pt[j] = PAGE_MASK__FAKE;
  }
  kernel_pt[0xB8] = 0xB8000 | PAGE_MASK__KERNEL; /* VGA */

  /* Set up rest of page-table hierarchy */

  pagetable pd = (pagetable)allocate_phys_page();
  memset(pd, 0x00, PAGE_4K_SIZE);
  pd[0] = ((uint64_t) kernel_pt) | PAGE_MASK__KERNEL;

  pagetable pdpt = (pagetable)allocate_phys_page();
  memset(pdpt, 0x00, PAGE_4K_SIZE);
  pdpt[0] = ((uint64_t) pd) | PAGE_MASK__KERNEL;

  pagetable pml4 = (pagetable)allocate_phys_page();
  memset(pml4, 0x00, PAGE_4K_SIZE);
  pml4[0] = ((uint64_t) pdpt) | PAGE_MASK__USER;

  /* For virtual-memory hacking */
  pml4[PAGE_NUM_ENTRIES-1] = ((uint64_t) pml4) | PAGE_MASK__KERNEL;

  return pml4;
}

/* Return a pagetable that contains enough information to set it as your
 * pagetable. */
pagetable new_pt (void) {
  pagetable pml4_phys = (pagetable)allocate_phys_page();
  pagetable pml4_virt = (pagetable)LOC_TEMP_PT; /* Arbitrary otherwise unmapped location */
  pagetable pdpt_phys = (pagetable)allocate_phys_page();
  pagetable pdpt_virt = (pagetable)LOC_TEMP_PT2; /* Arbitrary otherwise unmapped location */
  pagetable pd_phys = (pagetable)allocate_phys_page();
  pagetable pd_virt = (pagetable)LOC_TEMP_PT3; /* Arbitrary otherwise unmapped location */

  map_page((uint64_t)pml4_phys, (uint64_t)pml4_virt, PAGE_MASK__KERNEL);
  map_page((uint64_t)pdpt_phys, (uint64_t)pdpt_virt, PAGE_MASK__KERNEL);
  map_page((uint64_t)pd_phys, (uint64_t)pd_virt, PAGE_MASK__KERNEL);

  memset(pml4_virt, 0x00, PAGE_4K_SIZE);
  memset(pdpt_virt, 0x00, PAGE_4K_SIZE);
  memset(pd_virt, 0x00, PAGE_4K_SIZE);

  pd_virt[0] = ((uint64_t) kernel_pt) | PAGE_MASK__KERNEL;
  pdpt_virt[0] = ((uint64_t) pd_phys) | PAGE_MASK__KERNEL;
  pml4_virt[0] = ((uint64_t) pdpt_phys) | PAGE_MASK__USER;

  pml4_virt[PAGE_NUM_ENTRIES-1] = ((uint64_t) pml4_phys) | PAGE_MASK__KERNEL;

  unmap_page((uint64_t)pml4_virt);
  unmap_page((uint64_t)pdpt_virt);
  unmap_page((uint64_t)pd_virt);

  return pml4_phys;
}

int map_page (uint64_t phys, uint64_t virt, unsigned int mode) {
  if (phys & PAGE_4K_MASK) {
    return -1;
  }
  if (virt & PAGE_4K_MASK) {
    return -1;
  }

  pagetable pml4 = (pagetable) PAGE_VIRT_PML4_OF(virt);
  pagetable pdpt = (pagetable) PAGE_VIRT_PDPT_OF(virt);
  pagetable pd = (pagetable) PAGE_VIRT_PD_OF(virt);
  pagetable pt = (pagetable) PAGE_VIRT_PT_OF(virt);

  if (!(pml4[PAGE_HT_OF(virt)] & PAGE_MASK_PRESENT)) {
    pml4[PAGE_HT_OF(virt)] = ((uint64_t) allocate_phys_page()) | PAGE_MASK__USER;
    memset(pdpt, 0x00, PAGE_4K_SIZE);
  }
  if (!(pdpt[PAGE_1G_OF(virt)] & PAGE_MASK_PRESENT)) {
    pdpt[PAGE_1G_OF(virt)] = ((uint64_t) allocate_phys_page()) | PAGE_MASK__USER;
    memset(pd, 0x00, PAGE_4K_SIZE);
  }
  if (!(pd[PAGE_2M_OF(virt)] & PAGE_MASK_PRESENT)) {
    pd[PAGE_2M_OF(virt)] = ((uint64_t) allocate_phys_page()) | PAGE_MASK__USER;
    memset(pt, 0x00, PAGE_4K_SIZE);
  }

  pt[PAGE_4K_OF(virt)] = phys | mode;
  return 0;
}

int unmap_page (uint64_t virt) {
  if (virt & PAGE_4K_MASK) {
    return -1;
  }

  pagetable pml4 = (pagetable) PAGE_VIRT_PML4_OF(virt);
  pagetable pdpt = (pagetable) PAGE_VIRT_PDPT_OF(virt);
  pagetable pd = (pagetable) PAGE_VIRT_PD_OF(virt);
  pagetable pt = (pagetable) PAGE_VIRT_PT_OF(virt);

  if (!(pml4[PAGE_HT_OF(virt)] & PAGE_MASK_PRESENT)) {
    return 0;
  }
  if (!(pdpt[PAGE_1G_OF(virt)] & PAGE_MASK_PRESENT)) {
    return 0;
  }
  if (!(pd[PAGE_2M_OF(virt)] & PAGE_MASK_PRESENT)) {
    return 0;
  }

  pt[PAGE_4K_OF(virt)] = PAGE_MASK__FAKE;
  return 0;
}

int map_new_page (uint64_t virt, unsigned int mode) {
  return map_page((uint64_t)allocate_phys_page(), virt, mode);
}
