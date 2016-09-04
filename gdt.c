#include "gdt.h"

#include "util.h"

struct {
  unsigned int reserved1;
  unsigned long long rsp[3];
  unsigned long long ist[8]; /* ist[0] is reserved */
  unsigned short reserved3[5];
  unsigned short iomapbase;
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

char gdt[][8] = {
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
  long long base = (long long) &tss;
  int limit = sizeof(tss);
  gdt[10][0] = (char)(limit);
  gdt[10][1] = (char)(limit >> 8);
  gdt[10][2] = (char)(base);
  gdt[10][3] = (char)(base >> 8);
  gdt[10][4] = (char)(base >> 16);
  gdt[10][5] = 0x89;
  gdt[10][6] = (char)(0x40 | (limit >> 16));
  gdt[10][7] = (char)(base >> 24);
  gdt[11][0] = (char)(base >> 32);
  gdt[11][1] = (char)(base >> 40);
  gdt[11][2] = (char)(base >> 48);
  gdt[11][3] = (char)(base >> 56);
  memset((char *)&tss, sizeof(tss), 0);
  /* Set up TSS */
  tss.iomapbase = sizeof(tss); /* Disable IO map */
  /* Load TSS offset */
  unsigned short index = 0x50;
  __asm__("ltr %0" :: "r"(index));

}

void set_new_rsp (void *rsp) {
  tss.rsp[0] = (unsigned long long)rsp;
}

