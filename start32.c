/* This file runs in 32-bit mode.  To make sure we don't try and call 64-bit
 * code, don't include any headers. Here are the fake "imports" */

/* Data that we must access from here */

extern unsigned long long memtop;

extern char gdt[][8];

/* Useful constants */

#define PAGE_NUM_ENTRIES 512
#define PAGE_MASK_KERNEL 0x00000003
#define PAGE_MASK_SIZE   0x00000080

/* Functions available in start32-asm.s */

void initialize_segment_selectors(void);
void enable_ia32e(void);

/* End "imports" */

typedef unsigned long long pagetable[PAGE_NUM_ENTRIES] __attribute__((aligned(0x1000)));

pagetable bad_pml4;
pagetable bad_pdpt;

void enter_long_mode (int magic, unsigned int *mboot_struct) {
  /* Check multiboot magic */
  if (magic != 0x2BADB002) {
    __asm__("hlt");
  }

  /* Initialize memory allocator, while we still have easy access to the
   * multiboot information */
  memtop = ((unsigned long long) mboot_struct[2]) * 1024;
  memtop += 0x100000;
  memtop &= 0xFFF;

  /* Insert GDT.  It's not yet fully initialized, so the TSS descriptor will
   * not be correct.  This is all right, because we're not enabling
   * interrupts yet.  Don't call LTR to load the segment selecter, either--we
   * have to be in 64-bit mode to do that. */
  const struct {
    unsigned short size;
    void *base;
  } __attribute__((packed)) GDT_addr = {
    0x60,
    gdt,
  };
  __asm__("lgdt %0" : : "m"(GDT_addr) : );

  /* Start using our new GDT */
  initialize_segment_selectors();

  /* Create a bad, provisional page table */
  bad_pml4[PAGE_NUM_ENTRIES-1] = (unsigned int)(&bad_pml4[0]) | PAGE_MASK_KERNEL;
  bad_pml4[0] = (unsigned int)(&bad_pdpt[0]) | PAGE_MASK_KERNEL;
  bad_pdpt[0] = PAGE_MASK_KERNEL | PAGE_MASK_SIZE;

  /* Enable PAE */
  unsigned int cr4;
  __asm__("mov %%cr4,%0" : "=r"(cr4) : : );
  cr4 |= 0x00000020;
  __asm__("mov %0,%%cr4" : : "r"(cr4) : );

  /* Load page table */
  __asm__("mov %0,%%cr3" : : "r"(&bad_pml4) : );

  /* Enable long mode */
  enable_ia32e();

  /* Enable paging */
  unsigned int cr0;
  __asm__("mov %%cr0,%0" : "=r"(cr0) : : );
  cr0 |= 0x80000000;
  __asm__("mov %0,%%cr0" : : "r"(cr0) : );
}
