#ifndef __SILVOS_USERLAND_H
#define __SILVOS_USERLAND_H

#define CALLER_SAVE_REGISTERS2 "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#define CALLER_SAVE_REGISTERS "rax", CALLER_SAVE_REGISTERS2

static inline void yield (void) {
  __asm__ volatile("int $0x36" ::: CALLER_SAVE_REGISTERS);
}

static inline void putch (char c) {
  __asm__ volatile("int $0x37" : "=a" (c) : "a" (c) : CALLER_SAVE_REGISTERS2);
}

static inline void exit (void) {
  __asm__ volatile("int $0x38" ::: CALLER_SAVE_REGISTERS);
}

static inline char getch (void) {
  char c;
  __asm__ volatile("int $0x39" : "=a" (c) :: CALLER_SAVE_REGISTERS2);
  return c;
}

static inline int read (long long sector, char dest[512]) {
  int out;
  __asm__ volatile("int $0x3A;"
                   : "=a" (out)
                   : "a" (sector), "b" (dest)
                   : CALLER_SAVE_REGISTERS2, "memory");
  return out;
}

static inline int write (long long sector, const void *src) {
  int out;
  __asm__ volatile("int $0x3B;"
                   : "=a" (out)
                   : "a" (sector), "b" (src)
                   : CALLER_SAVE_REGISTERS2, "memory");
  return out;
}

#endif
