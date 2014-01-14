#include "isr.h"
#include "util.h"
#include "alloc.h"

struct IDT_entry {
  unsigned short offset_low;
  unsigned short selector;
  unsigned char zero;
  unsigned char type_attr;
  unsigned short offset_high;
} __attribute__ ((packed));

struct IDT_addr {
  unsigned short size;
  unsigned int base;
} __attribute__ ((packed));

struct IDT_entry *idt;

void create_idt () {
  idt = (struct IDT_entry *)allocate_phys_page();
}

void register_isr (unsigned char num, void (*handler)(void)) {
  idt[num].type_attr = 0x8E; /* present 32-bit interrupt gate in ring 3 */
  idt[num].offset_high = 0xFFFF & ((unsigned int) handler >> 16);
  idt[num].offset_low = 0xFFFF & ((unsigned int) handler);
  idt[num].zero = 0;
  idt[num].selector = 8;
}

void insert_idt (void) {
  const struct IDT_addr k = { 0x1000, (unsigned int) idt };
  __asm__("lidt %0" : : "m"(k) : );
}

unsigned int get_idt (void) {
  struct IDT_addr k;
  __asm__("sidt %0" : "=m"(k) : :);
  return k.base;
}
