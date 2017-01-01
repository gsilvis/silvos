#include "page.h"

#include "memory-map.h"
#include "page-constants.h"

#include "alloc.h"
#include "pagemap.h"
#include "threads.h"
#include "util.h"

#include <stdint.h>

pagetable kernel_pdpt;

void insert_pt (pagetable pt) {
  __asm__("mov %0,%%cr3" : : "r"((pagetable)virt_to_phys((uint64_t)pt)) : );
}


extern int _end;

pagetable initial_pt (void) {
  /* Set up the shared kernel pt */
  kernel_pdpt = (pagetable)allocate_phys_page();
  for (uint64_t j = 0x00; j < PAGE_PT_NUM_ENTRIES - 2; j++) {
    kernel_pdpt[j] = (j * PAGE_1G_SIZE) | PAGE_MASK__KERNEL | PAGE_MASK_SIZE;
  }
  kernel_pdpt[PAGE_PT_NUM_ENTRIES-2] = PAGE_MASK__KERNEL | PAGE_MASK_SIZE;
  kernel_pdpt[PAGE_PT_NUM_ENTRIES-1] = PAGE_1G_SIZE | PAGE_MASK__KERNEL | PAGE_MASK_SIZE;
  return new_pt();
}

/* Return a pagetable that contains enough information to set it as your
 * pagetable. */
pagetable new_pt (void) {
  pagetable pml4 = (pagetable)allocate_phys_page();
  memset(pml4, 0x00, PAGE_4K_SIZE);
  pml4[PAGE_PT_SELF_MAP] = virt_to_phys((uint64_t)pml4) | PAGE_MASK__KERNEL;
  pml4[PAGE_PT_NUM_ENTRIES-1] = virt_to_phys((uint64_t)kernel_pdpt) | PAGE_MASK__KERNEL;
  return pml4;
}

/* Get a pointer to the pt entry for a specified virtual address.
 *
 * If mode contains 'PAGE_MASK_PRESENT', create intermediate page table entries
 * along the way, with that mode.
 *
 * Otherwise, if an intermediate page table entry is missing, return NULL
 */
static uint64_t *get_page_entry (uint64_t virt, uint64_t mode) {
  pagetable pml4 = (pagetable) PAGE_VIRT_PML4_OF(virt);
  pagetable pdpt = (pagetable) PAGE_VIRT_PDPT_OF(virt);
  pagetable pd = (pagetable) PAGE_VIRT_PD_OF(virt);
  pagetable pt = (pagetable) PAGE_VIRT_PT_OF(virt);

  if (mode & PAGE_MASK_PRESENT) {
    if (!(pml4[PAGE_HT_OF(virt)] & PAGE_MASK_PRESENT)) {
      pml4[PAGE_HT_OF(virt)] = virt_to_phys((uint64_t)allocate_phys_page()) | mode;
      memset(pdpt, 0x00, PAGE_4K_SIZE);
    }
    if (!(pdpt[PAGE_1G_OF(virt)] & PAGE_MASK_PRESENT)) {
      pdpt[PAGE_1G_OF(virt)] = virt_to_phys((uint64_t)allocate_phys_page()) | mode;
      memset(pd, 0x00, PAGE_4K_SIZE);
    }
    if (!(pd[PAGE_2M_OF(virt)] & PAGE_MASK_PRESENT)) {
      pd[PAGE_2M_OF(virt)] = virt_to_phys((uint64_t)allocate_phys_page()) | mode;
      memset(pt, 0x00, PAGE_4K_SIZE);
    }
  } else {
    if (!(pml4[PAGE_HT_OF(virt)] & PAGE_MASK_PRESENT)) {
      return NULL;
    }
    if (!(pdpt[PAGE_1G_OF(virt)] & PAGE_MASK_PRESENT)) {
      return NULL;
    }
    if (!(pd[PAGE_2M_OF(virt)] & PAGE_MASK_PRESENT)) {
      return NULL;
    }
  }

  return &pt[PAGE_4K_OF(virt)];
}

int unmap_page (uint64_t virt) {
  if (virt & PAGE_4K_MASK) {
    return -1;
  }
  pagemap *pm = &running_tcb->pm;
  int ret;
  if ((ret = pm_remove_ent(pm, virt))) {
    return ret;
  }

  uint64_t *pt_entry = get_page_entry(virt, PAGE_MASK__FAKE);
  if (!pt_entry) {
    /* TODO: Bug-logging if we remove a page that's not mapped. */
    return -1;
  }
  *pt_entry = PAGE_MASK__FAKE;
  return 0;
}

int map_new_page (uint64_t virt, uint64_t mode) {
  if (virt & PAGE_4K_MASK) {
    return -1;
  }
  pagemap *pm = &running_tcb->pm;
  /* Don't map the same page twice */
  if (pm_is_mapped(pm, virt)) {
    return -2;
  }
  if (pm->num_entries == NUMMAPS) {
    return -3;
  }
  uint64_t *pt_entry = get_page_entry(virt, PAGE_MASK__USER);
  *pt_entry = virt_to_phys((uint64_t)allocate_phys_page()) | mode;
  /* We checked that the pagemap wasn't full before, so this can't fail. */
  pm_add_ent(pm, virt, *pt_entry);
  return 0;
}


void clone_pagemap (pagemap *dst, pagemap *src) {
  /* TODO: Copy-on-write. */
  for (uint8_t i = 0; i < src->num_entries; ++i) {
    void *new_page = allocate_phys_page();
    memcpy(new_page, (void *)phys_to_virt(PAGE_PADDR_FROM_ENTRY(src->entries[i].phys)), PAGE_4K_SIZE);
    dst->entries[i].virt = src->entries[i].virt;
    dst->entries[i].phys = virt_to_phys((uint64_t) new_page) | (src->entries[i].phys & PAGE_4K_MASK);
  }
  dst->num_entries = src->num_entries;
}

void apply_pagemap (void) {
  pagemap *pm = &running_tcb->pm;
  for (uint8_t i = 0; i < pm->num_entries; ++i) {
    pagemap_ent ent = pm->entries[i];
    uint64_t *pt_entry = get_page_entry(ent.virt, PAGE_MASK__USER);
    *pt_entry = ent.phys;
  }
}
