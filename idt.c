#include "isr.h"
#include "util.h"
#include "alloc.h"
#include "page.h"

struct IDT_entry {
  unsigned short offset_low;
  unsigned short selector;
  unsigned char ist;
  unsigned char type_attr;
  unsigned short offset_mid;
  unsigned int offset_high;
  unsigned int reserved;
} __attribute__ ((packed));


struct IDT_entry *idt;

void create_idt () {
  idt = (struct IDT_entry *)allocate_phys_page();
  map_page((unsigned long long)idt, (unsigned long long)idt, PAGE_MASK__KERNEL);
}

void register_isr (unsigned char num, void (*handler)(void)) {
  unsigned long long offset = (unsigned long long)handler;
  idt[num].type_attr = 0xEE; /* present 64-bit interrupt gate in ring 3 */
  /* 0x8E for ring0 only, EE for ring3 */
  idt[num].offset_high = (unsigned int)(offset >> 32);
  idt[num].offset_mid = (unsigned short)(offset >> 16);
  idt[num].offset_low = (unsigned short)(offset);
  idt[num].ist = 0; /* Disable IST feature */
  idt[num].reserved = 0;
  idt[num].selector = 0x40;
}

void insert_idt (void) {
  const struct {
    unsigned short size;
    void *base;
  } __attribute__ ((packed)) IDT_addr = {
    0x1000,
    idt,
  };
  __asm__("lidt %0" : : "m"(IDT_addr) : );
}
