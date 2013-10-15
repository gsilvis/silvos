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

#endif
