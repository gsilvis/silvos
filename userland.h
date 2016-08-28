#ifndef __SILVOS_USERLAND_H
#define __SILVOS_USERLAND_H

inline void yield (void) {
  __asm__ volatile("int $0x36" :: "r"(0x36));
}

inline void putch (char c) {
  __asm__ volatile("mov %[input],%%al;"
                   "int $0x37;"
                   :: [input] "m" (c): "%eax");
}

#endif
