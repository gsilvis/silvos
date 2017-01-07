#include <stdint.h>

#define MAP_OFFSET 0xFFFFFFFF80000000

void memcpy (uint64_t dest, uint64_t src, uint64_t n) {
  uint8_t *d = (uint8_t *)dest;
  uint8_t *s = (uint8_t *)src;
  for (uint64_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
}

void memset (uint64_t addr, uint8_t c, uint64_t n) {
  uint8_t *a = (uint8_t *)addr;
  for (uint64_t i = 0; i < n; i++) {
    a[i] = c;
  }
}

void __attribute__ ((noreturn)) fail (void) {
  while (1) {
    __asm__("hlt");
  }
}

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t string;
  uint32_t unused;
} multiboot_module;

uint64_t relocate_kernel (uint32_t mboot_struct_addr) {
  /*
   * Assumption:  All multiboot modules and tables are below where the kernel
   * asks to be loaded.
   */
  uint32_t *mboot_struct = (uint32_t *)(uint64_t)mboot_struct_addr;
  if (mboot_struct[5] == 0) {
    fail();
  }
  multiboot_module *mod_list = (multiboot_module *)(uint64_t)mboot_struct[6];

  /* Find out where we're copying the kernel */
  uint32_t *kernel = (uint32_t *)(uint64_t)mod_list[0].start;
  uint16_t i;
  for (i = 0; i < 2048; i++) {
    if (kernel[i] == 0x1BADB002) {
      if (0 == 0x1BADB002 + kernel[i+1] + kernel[i+2]) {
        break; /* Found multiboot magic with valid checksum */
      }
    }
  }
  if (i == 2048) {
    fail(); /* No multiboot header found. */
  }
  if (kernel[i+1] & 0x00007FFC) {
    fail(); /* Unknown/unsupported mandatory flag */
  }
  if (!(kernel[i+1] & 0x00008000)) {
    fail(); /* We do not support parsing ELF */
  }

  uint32_t header_addr = kernel[i+3];
  uint32_t load_addr = kernel[i+4];
  uint32_t load_end_addr = kernel[i+5];
  uint32_t bss_end_addr = kernel[i+6];
  uint32_t entry_addr = kernel[i+7];

  /* Move the kernel */
  memcpy(load_addr,
         ((uint64_t)&kernel[i]) + load_addr - header_addr,
         load_end_addr - load_addr);
  memset(load_end_addr, 0, bss_end_addr - load_end_addr);

  /* Fix up the list of modules */
  mboot_struct[5]--;
  mboot_struct[6] += sizeof(multiboot_module);

  /* Return the entry point. */
  return entry_addr + MAP_OFFSET;
}
