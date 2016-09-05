#include "gdt.h"

#include "util.h"

#include <stdint.h>
#include <stddef.h>

struct {
  uint32_t reserved1;
  uint64_t rsp[3];
  uint64_t ist[8]; /* ist[0] is reserved */
  uint16_t reserved3[5];
  uint16_t iomapbase;
} __attribute__((packed)) tss;

/* Useful stack segment selectors:
 *        Ring 0    Ring 3
 * Data   0x10      0x1B
 * 16-b   0x20      0x2B
 * 32-b   0x30      0x3B
 * 64-b   0x40      0x4B
 *
 * The data and 32-bit segments can be used in both protected mode and ia32-e
 * mode.  The 16-bit and 64-bit segments can only be used in ia32-e mode.
 */

uint8_t gdt[][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* Null */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* Unused */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00}, /* ring 0 data */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF2, 0xCF, 0x00}, /* ring 3 data */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0x80, 0x00}, /* 16-b ring 0 code */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x80, 0x00}, /* 16-b ring 3 code */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00}, /* 32-b ring 0 code */
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFA, 0xCF, 0x00}, /* 32-b ring 3 code */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x9B, 0xA0, 0x00}, /* 64-b ring 0 code */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0xFB, 0xA0, 0x00}, /* 64-b ring 3 code */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* TSS, lower half */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* TSS, upper half */
};

void initialize_gdt (void) {
  /* Set up TSS descriptor */
  uint64_t base = (uint64_t) &tss;
  size_t limit = sizeof(tss);
  gdt[10][0] = (uint8_t)(limit);
  gdt[10][1] = (uint8_t)(limit >> 8);
  gdt[10][2] = (uint8_t)(base);
  gdt[10][3] = (uint8_t)(base >> 8);
  gdt[10][4] = (uint8_t)(base >> 16);
  gdt[10][5] = 0x89;
  gdt[10][6] = (uint8_t)(0x40 | (limit >> 16));
  gdt[10][7] = (uint8_t)(base >> 24);
  gdt[11][0] = (uint8_t)(base >> 32);
  gdt[11][1] = (uint8_t)(base >> 40);
  gdt[11][2] = (uint8_t)(base >> 48);
  gdt[11][3] = (uint8_t)(base >> 56);
  memset(&tss, sizeof(tss), 0);
  /* Set up TSS */
  tss.iomapbase = sizeof(tss); /* Disable IO map */
  /* Load TSS offset */
  uint16_t index = 0x50;
  __asm__("ltr %0" :: "r"(index));

}

void set_new_rsp (void *rsp) {
  tss.rsp[0] = (uint64_t)rsp;
}

