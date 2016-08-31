#ifndef __SILVOS_USERLAND_H
#define __SILVOS_USERLAND_H

static inline void yield (void) {
  __asm__ volatile("int $0x36" :: );
}

static inline void putch (char c) {
  __asm__ volatile("mov %[input],%%al;"
                   "int $0x37;"
                   :: [input] "m" (c): "%eax");
}

static inline void exit (void) {
  __asm__ volatile("int $0x38" :: );
}

static inline char getch (void) {
  char c;
  __asm__ volatile("int $0x39;"
                   "mov %%al,%[output]"
                   : [output] "=m" (c) :: "%eax");
  return c;
}

#endif
