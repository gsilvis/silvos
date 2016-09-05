#ifndef __SILVOS_UTIL_H
#define __SILVOS_UTIL_H

#include <stdint.h>
#include <stddef.h>

void memset (void *ptr, char byte, size_t count);
void memcpy (void *dest, const void *src, size_t count);
void __attribute__ ((noreturn)) panic (const char *s);
void blab (void);


typedef struct {
  void *rip;
  void *rsp;
  void *rbx;
  void *rbp;
  void *r12;
  void *r13;
  void *r14;
  void *r15;
} __attribute__ ((packed)) jmp_buf[1];

int __attribute__ ((returns_twice)) setjmp (jmp_buf buf);
void __attribute__ ((noreturn)) longjmp (jmp_buf buf, int val);


static inline void hlt (void) {
  __asm__("hlt");
}

static inline void cli (void) {
  __asm__("cli");
}

static inline void sti (void) {
  __asm__("sti");
}

static inline void outb (uint8_t port, uint8_t data) {
  __asm__("mov %1, %%al;\n\t"
          "out %%al, %0" : : "i"(port), "r"(data) : "%eax");
}

static inline uint8_t inb (uint8_t port) {
  uint8_t data;
  __asm__ volatile("in %1, %%al\n\t"
                   "mov %%al, %0" : "=r"(data) : "i"(port) : "%eax", "%edx");
  return data;
}


#endif
