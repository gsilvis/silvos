#include "idt.h"

#include "memory-map.h"

#include "util.h"
#include "alloc.h"
#include "page.h"

struct IDT_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attr;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t reserved;
} __attribute__ ((packed));


struct IDT_entry *idt;

void create_idt () {
  idt = (struct IDT_entry *)allocate_phys_page();
  const struct {
    uint16_t size;
    void *base;
  } __attribute__ ((packed)) IDT_addr = {
    0x1000,
    idt,
  };
  __asm__("lidt %0" : : "m"(IDT_addr) : );
}

static void register_isr_internal (uint8_t num,
                                   void (*handler)(void),
                                   uint8_t ist) {
  uint64_t offset = (uint64_t)handler;
  idt[num].offset_high = (uint32_t)(offset >> 32);
  idt[num].offset_mid = (uint16_t)(offset >> 16);
  idt[num].offset_low = (uint16_t)(offset);
  idt[num].ist = ist; /* ist = 0 disables */
  idt[num].reserved = 0;
  idt[num].selector = 0x40;
}

void register_isr (uint8_t num,
                   void (*handler)(void),
                   uint8_t ist) {
  register_isr_internal(num, handler, ist);
  idt[num].type_attr = 0x8E;
  /* present 64-bit interrupt gate only callable from ring 0. */
}

void register_user_isr (uint8_t num,
                        void (*handler)(void),
                        uint8_t ist) {
  register_isr_internal(num, handler, ist);
  idt[num].type_attr = 0xEE;
  /* present 64-bit interrupt callable from ring 3. */
}
