#include <stdint.h>

uint8_t gdt[][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* Null */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00}, /* ring 0 data */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00}, /* 32-b ring 0 code */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 0xA0, 0x00}, /* 64-b ring 0 code */
};

#define PAGE_NUM_ENTRIES 512
#define PAGE_MASK_KERNEL 0x00000003
#define PAGE_MASK_SIZE   0x00000080

typedef uint64_t pagetable[PAGE_NUM_ENTRIES] __attribute__((aligned(0x1000)));

pagetable bad_pml4;
pagetable low_pdpt;
pagetable high_pdpt;
pagetable bad_pds[2];

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

/* We map the bottom 2GB of memory to three locations: zero, -512GB, and -2GB. */
void init_page_table (void) {
  bad_pml4[0] = (uint32_t)(&low_pdpt[0]) | PAGE_MASK_KERNEL;
  bad_pml4[511] = (uint32_t)(&high_pdpt[0]) | PAGE_MASK_KERNEL;

  low_pdpt[0] = (uint32_t)(&bad_pds[0][0]) | PAGE_MASK_KERNEL;
  low_pdpt[1] = (uint32_t)(&bad_pds[1][0]) | PAGE_MASK_KERNEL;
  high_pdpt[0] = (uint32_t)(&bad_pds[0][0]) | PAGE_MASK_KERNEL;
  high_pdpt[1] = (uint32_t)(&bad_pds[1][0]) | PAGE_MASK_KERNEL;
  high_pdpt[510] = (uint32_t)(&bad_pds[0][0]) | PAGE_MASK_KERNEL;
  high_pdpt[511] = (uint32_t)(&bad_pds[1][0]) | PAGE_MASK_KERNEL;

  for (uint64_t i = 0; i < PAGE_NUM_ENTRIES; i++) {
    bad_pds[0][i] = (i << 21) | PAGE_MASK_KERNEL | PAGE_MASK_SIZE;
    bad_pds[1][i] = ((i + PAGE_NUM_ENTRIES) << 21) | PAGE_MASK_KERNEL | PAGE_MASK_SIZE;
  }
}
