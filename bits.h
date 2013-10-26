#ifndef __SILVOS_BITS_H
#define __SILVOS_BITS_H

static inline void hlt (void) {
  __asm__("hlt");
}

static inline void cli (void) {
  __asm__("cli");
}

static inline void sti (void) {
  __asm__("sti");
}

static inline void outb (unsigned char port, unsigned char data) {
  __asm__("mov %1, %%al;\n\t"
          "out %%al, %0" : : "i"(port), "r"(data) : "%eax");
}

static inline unsigned char inb (unsigned char port) {
  unsigned char data;
  __asm__ volatile("in %1, %%al\n\t"
                   "mov %%al, %0" : "=r"(data) : "i"(port) : "%eax", "%edx");
  return data;
}

#endif
