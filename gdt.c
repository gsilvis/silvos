#include "gdt.h"
#include "util.h"

void initialize_segment_selectors(void); /* in gdt-asm.s */

const char gdt[][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* Descriptor 0 is unusable */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00}, /* Code: 0x00000000 to 0xFFFFFFFF, type 0x9A */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00}, /* Data: 0x00000000 to 0xFFFFFFFF, type 0x92 */
  /* Put a TSS descriptor here */
};

void insert_gdt (void) {
  const struct {
    unsigned short size;
    void *base;
  } __attribute__((packed)) GDT_addr = {
    0x18,
    gdt,
  };
  __asm__("lgdt %0" : : "m"(GDT_addr) : );
  initialize_segment_selectors();
}

