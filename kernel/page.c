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
  pml4[PAGE_PT_NUM_ENTRIES-1] = virt_to_phys((uint64_t)kernel_pdpt) | PAGE_MASK__KERNEL;
  return pml4;
}

/* Dereference one step of a page table.  If mode contains 'PAGE_MASK_PRESENT',
 * create that level if it does not exist.  Else, return NULL if it does not exist */
static pagetable dereference_page_table (pagetable pt, uint64_t index, uint64_t mode) {
  if (pt[index] & PAGE_MASK_PRESENT) {
    return (pagetable)phys_to_virt(PAGE_PADDR_FROM_ENTRY(pt[index]));
  }
  if (!(mode & PAGE_MASK_PRESENT)) {
    return NULL;
  }
  void *page_phys = allocate_phys_page();
  memset(page_phys, 0x00, PAGE_4K_SIZE);
  pt[index] = virt_to_phys((uint64_t)page_phys) | mode;
  return (pagetable)page_phys;
}

/* Get a pointer to the pt entry for a specified virtual address.
 *
 * If mode contains 'PAGE_MASK_PRESENT', create intermediate page table entries
 * along the way, with that mode.
 *
 * Otherwise, if an intermediate page table entry is missing, return NULL
 */
static uint64_t *get_page_entry (pagetable pml4, uint64_t virt, uint64_t mode) {
  pagetable pdpt = dereference_page_table(pml4, PAGE_HT_OF(virt), mode);
  if (!pdpt) {
    return NULL;
  }
  pagetable pd = dereference_page_table(pdpt, PAGE_1G_OF(virt), mode);
  if (!pd) {
    return NULL;
  }
  pagetable pt = dereference_page_table(pd, PAGE_2M_OF(virt), mode);
  if (!pt) {
    return NULL;
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

  uint64_t *pt_entry = get_page_entry(running_tcb->pt, virt, PAGE_MASK__FAKE);
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
  uint64_t *pt_entry = get_page_entry(running_tcb->pt, virt, PAGE_MASK__USER);
  *pt_entry = virt_to_phys((uint64_t)allocate_phys_page()) | mode;
  /* We checked that the pagemap wasn't full before, so this can't fail. */
  pm_add_ent(pm, virt, *pt_entry);
  return 0;
}

static void copy_page (pagetable dest_pml4, pagetable src_pml4, uint64_t virt) {
  /* TODO: COW.  Note that COWing a COW'd page is subtle */
  uint64_t *src_entry = get_page_entry(src_pml4, virt, PAGE_MASK__FAKE);
  if (!src_entry) {
    return;
  }
  uint64_t entry = *src_entry;
  if (!(entry & PAGE_MASK_PRESENT)) {
    return;
  }
  void *new_page = allocate_phys_page();
  memcpy(new_page, (void *)phys_to_virt(PAGE_PADDR_FROM_ENTRY(entry)), PAGE_4K_SIZE);
  uint64_t *dest_entry = get_page_entry(dest_pml4, virt, PAGE_MASK__USER);
  *dest_entry = virt_to_phys((uint64_t)new_page) | PAGE_FLAGS_FROM_ENTRY(entry);
}

pagetable duplicate_pagetable (pagetable src_pml4) {
  pagetable pml4 = new_pt();
  for (uint16_t a = 0; a < 511; a++) {
    pagetable src_pdpt = dereference_page_table(src_pml4, a, 0);
    if (!src_pdpt) {
      continue;
    }
    for (uint16_t b = 0; b < 512; b++) {
      pagetable src_pd = dereference_page_table(src_pdpt, b, 0);
      if (!src_pd) {
        continue;
      }
      for (uint16_t c = 0; c < 512; c++) {
	pagetable src_pt = dereference_page_table(src_pd, c, 0);
	if (!src_pt) {
	  continue;
	}
        for (uint16_t d = 0; d < 512; d++) {
          uint64_t addr = a*PAGE_HT_SIZE + b*PAGE_1G_SIZE +
                          c*PAGE_2M_SIZE + d*PAGE_4K_SIZE;
          copy_page(pml4, src_pml4, addr);
        }
      }
    }
  }
  return pml4;
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
    uint64_t *pt_entry = get_page_entry(running_tcb->pt, ent.virt, PAGE_MASK__USER);
    *pt_entry = ent.phys;
  }
}
