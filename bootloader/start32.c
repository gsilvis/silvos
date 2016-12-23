/* This file runs in 32-bit mode.  To make sure we don't try and call 64-bit
 * code, don't include any SILVOS headers. Here are the fake "imports" */

/* Standard library imports */
#include <stdint.h>

/* Data that we must access from here */

uint8_t gdt[][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* Null */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00}, /* ring 0 data */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00}, /* 32-b ring 0 code */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 0xA0, 0x00}, /* 64-b ring 0 code */
};

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

void check_magic (uint32_t magic) {
  if (magic != 0x2BADB002) {
    __asm__("hlt");
  }
}

void setup_gdt (void) {
  const struct {
    uint16_t size;
    void *base;
  } __attribute__((packed)) GDT_addr = {
    0x20,
    gdt,
  };
  __asm__("lgdt %0" : : "m"(GDT_addr) : );
}

void init_page_table (void) {
  bad_pml4[0] = (uint32_t)(&bad_pdpt[0]) | PAGE_MASK_KERNEL;
  bad_pml4[511] = (uint32_t)(&bad_pdpt[0]) | PAGE_MASK_KERNEL;
  for (uint64_t i = 0; i < PAGE_NUM_ENTRIES - 2; i++) {
    bad_pdpt[i] = (i << 30) | PAGE_MASK_KERNEL | PAGE_MASK_SIZE;
  }
  bad_pdpt[510] = (0 << 30) | PAGE_MASK_KERNEL | PAGE_MASK_SIZE;
  bad_pdpt[511] = (1 << 30) | PAGE_MASK_KERNEL | PAGE_MASK_SIZE;
}
