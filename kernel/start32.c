/* This file runs in 32-bit mode.  To make sure we don't try and call 64-bit
 * code, don't include any SILVOS headers. Here are the fake "imports" */

/* Standard library imports */
#include <stdint.h>

/* Data that we must access from here */

extern uint64_t memtop;

extern uint8_t gdt[][8];

/* Useful constants */

#define PAGE_NUM_ENTRIES 512
#define PAGE_MASK_KERNEL 0x00000003
#define PAGE_MASK_SIZE   0x00000080

/* Functions available in start32-asm.s */

void initialize_segment_selectors(void);
void enable_ia32e(void);

/* End "imports" */

typedef uint64_t pagetable[PAGE_NUM_ENTRIES] __attribute__((aligned(0x1000)));

pagetable bad_pml4;
pagetable bad_pdpt;

void enter_long_mode (uint32_t magic, uint32_t *mboot_struct) {
  /* Check multiboot magic */
  if (magic != 0x2BADB002) {
    __asm__("hlt");
  }

  /* Initialize memory allocator, while we still have easy access to the
   * multiboot information */
  memtop = ((uint64_t) mboot_struct[2]) * 1024;
  memtop += 0x100000; /* low memory */

  /* Insert GDT.  It's not yet fully initialized, so the TSS descriptor will
   * not be correct.  This is all right, because we're not enabling
   * interrupts yet.  Don't call LTR to load the segment selecter, either--we
   * have to be in 64-bit mode to do that. */
  const struct {
    uint16_t size;
    void *base;
  } __attribute__((packed)) GDT_addr = {
    0x60,
    gdt,
  };
  __asm__("lgdt %0" : : "m"(GDT_addr) : );

  /* Start using our new GDT */
  initialize_segment_selectors();

  /* Create a bad, provisional page table */
  bad_pml4[PAGE_NUM_ENTRIES-1] = (uint32_t)(&bad_pml4[0]) | PAGE_MASK_KERNEL;
  bad_pml4[0] = (uint32_t)(&bad_pdpt[0]) | PAGE_MASK_KERNEL;
  bad_pdpt[0] = PAGE_MASK_KERNEL | PAGE_MASK_SIZE;

  /* Enable PAE */
  uint32_t cr4;
  __asm__("mov %%cr4,%0" : "=r"(cr4) : : );
  cr4 |= 0x00000020;
  __asm__("mov %0,%%cr4" : : "r"(cr4) : );

  /* Load page table */
  __asm__("mov %0,%%cr3" : : "r"(&bad_pml4) : );

  /* Enable long mode */
  enable_ia32e();

  /* Enable paging */
  uint32_t cr0;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x80000000;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
}
