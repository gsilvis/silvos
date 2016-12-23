#include <stdint.h>

#define MAP_OFFSET 0xFFFFFFFFC0000000

void memmove (uint64_t dest, uint64_t src, uint64_t n) {
  uint8_t *d = (uint8_t *)dest;
  uint8_t *s = (uint8_t *)src;
  if (dest > src) {
    for (uint64_t i = 0; i < n; i++) {
      d[n-i-1] = s[n-i-1];
    }
  } else if (dest < src) {
    for (uint64_t i = 0; i < n; i++) {
      d[i] = s[i];
    }
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

uint64_t relocate_kernel (uint32_t mboot_struct_addr) {
  uint32_t *mboot_struct = (uint32_t *)(uint64_t)mboot_struct_addr;
  if (mboot_struct[5] == 0) {
    fail();
  }
  uint32_t *kernel_mod_struct = (uint32_t *)(uint64_t)mboot_struct[6];
  uint32_t *kernel = (uint32_t *)(uint64_t)kernel_mod_struct[0];
  for (uint16_t i = 0; i < 2048; i++) {
    if (kernel[i] == 0x1BADB002) {
      uint32_t flags = kernel[i+1];
      uint32_t check = kernel[i+2];
      if (0x1BADB002 + flags + check) {
	continue; /* Bad checksum */
      }
      if (flags & 0x00007FFC) {
        fail(); /* Unknown/unsupported mandatory flag */
      }
      if (!(flags & 0x00008000)) {
        fail(); /* We do not support parsing ELF */
      }
      uint32_t header_addr = kernel[i+3];
      uint32_t load_addr = kernel[i+4];
      uint32_t load_end_addr = kernel[i+5];
      uint32_t bss_end_addr = kernel[i+6];
      uint32_t entry_addr = kernel[i+7];
      memmove(load_addr,
              ((uint64_t)&kernel[i]) + load_addr - header_addr,
              load_end_addr - load_addr);
      memset(load_end_addr, 0, bss_end_addr - load_end_addr);
      return entry_addr + MAP_OFFSET;
    }
  }
  fail(); /* No multiboot header found. */
}
