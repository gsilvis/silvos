#include <stdint.h>

#define MAP_OFFSET 0xFFFFFFFF80000000

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

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t string;
  uint32_t unused;
} multiboot_module;

uint64_t relocate_kernel (uint32_t mboot_struct_addr) {
  /* Procedure:  Find out where the kernel will end up.  For each module that
   * overlaps that, copy it past the end.  Then move the kernel into place.
   * Finally, copy the displaced modules back to where the kernel used to be.
   *
   * Assumptions:  That the modules are all AFTER the multiboot header and the
   * module list.  That no one cares about module names.
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

  uint32_t old_kernel_loc = mod_list[0].start;
  uint32_t header_addr = kernel[i+3];
  uint32_t load_addr = kernel[i+4];
  uint32_t load_end_addr = kernel[i+5];
  uint32_t bss_end_addr = kernel[i+6];
  uint32_t entry_addr = kernel[i+7];

  /* First, copy all the modules the kernel will interfere with to after the
   * kernel. */
  uint32_t copy_to_addr = bss_end_addr;
  for (uint32_t j = 1; j < mboot_struct[5]; j++) {
    if (mod_list[j].start < bss_end_addr && mod_list[j].end > load_addr) {
      uint32_t length = mod_list[j].end - mod_list[j].start;
      memmove(copy_to_addr, mod_list[j].start, length);
      mod_list[j].start = copy_to_addr;
      mod_list[j].end = copy_to_addr + length;
    }
  }

  /* Move the kernel */
  memmove(load_addr,
          ((uint64_t)&kernel[i]) + load_addr - header_addr,
          load_end_addr - load_addr);
  memset(load_end_addr, 0, bss_end_addr - load_end_addr);

  /* Move the modules back */
  copy_to_addr = old_kernel_loc;
  for (uint32_t j = 1; j < mboot_struct[5]; j++) {
    if (mod_list[j].start >= bss_end_addr) {
      uint32_t length = mod_list[j].end - mod_list[j].start;
      memmove(copy_to_addr, mod_list[j].start, length);
      mod_list[j].start = copy_to_addr;
      mod_list[j].end = copy_to_addr + length;
    }
  }

  /* Fix up the list of modules */
  for (uint32_t j = 0; j < mboot_struct[5]; j++) {
    mod_list[j].string = 0; /* These got clobbered anyways */
  }
  mboot_struct[5]--;
  mboot_struct[6] += sizeof(multiboot_module);

  /* Return the entry point. */
  return entry_addr + MAP_OFFSET;
}
