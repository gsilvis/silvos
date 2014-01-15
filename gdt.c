#include "gdt.h"
#include "util.h"

void initialize_segment_selectors(void); /* in gdt-asm.s */

struct {
  unsigned short link;
  unsigned short unused1;
  unsigned long esp0;
  unsigned short ss0;
  unsigned short unused2;
  unsigned long esp1;
  unsigned short ss1;
  unsigned short unused3;
  unsigned long esp2;
  unsigned short ss2;
  unsigned short unused4;
  unsigned long cr3;
  unsigned long eip;
  unsigned long eflags;
  unsigned long eax;
  unsigned long ecx;
  unsigned long edx;
  unsigned long ebx;
  unsigned long esp;
  unsigned long ebp;
  unsigned long esi;
  unsigned long edi;
  unsigned short es;
  unsigned short unused5;
  unsigned short cs;
  unsigned short unused6;
  unsigned short ss;
  unsigned short unused7;
  unsigned short ds;
  unsigned short unused8;
  unsigned short fs;
  unsigned short unused9;
  unsigned short gs;
  unsigned short unused10;
  unsigned short ldt;
  unsigned short unused11;
  unsigned short trap;
  unsigned short iomap;
} __attribute__((packed)) tss;

/* Six entries: Null descriptor, ring 0 code, ring 0 data, ring 3 code, ring 4
   data, TSS */

char gdt[][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00},
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00},
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFA, 0xCF, 0x00},
  {0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF2, 0xCF, 0x00},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* fix below */
};

void initialize_gdt (void) {
  int base = (int) &tss;
  int limit = sizeof(tss);
  gdt[5][0] = (char)(limit);
  gdt[5][1] = (char)(limit >> 8);
  gdt[5][2] = (char)(base);
  gdt[5][3] = (char)(base >> 8);
  gdt[5][4] = (char)(base >> 16);
  gdt[5][5] = 0x89;
  gdt[5][6] = (char)(0x40 | (limit >> 16));
  gdt[5][7] = (char)(base >> 24);
  memset((char *)&tss, sizeof(tss), 0);
  tss.ss0 = 0x10;
  tss.iomap = sizeof(tss);
}

void set_new_esp (void *esp) {
  tss.esp0 = (unsigned long)esp;
}

void insert_gdt (void) {
  const struct {
    unsigned short size;
    void *base;
  } __attribute__((packed)) GDT_addr = {
    0x30,
    gdt,
  };
  __asm__("lgdt %0" : : "m"(GDT_addr) : );
  initialize_segment_selectors();
  unsigned short index = 0x28;
  __asm__("ltr %0" :: "r"(index));
}

