#ifndef __SILVOS_UTIL_H
#define __SILVOS_UTIL_H

#include <stdint.h>
#include <stddef.h>

void memset (void *ptr, char byte, size_t count);
void memcpy (void *dest, const void *src, size_t count);
void __attribute__ ((noreturn)) panic (const char *s);
void blab (void);


typedef struct {
  /* %rip, %rsp, and all callee-save registers */
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

static inline void outb (uint16_t port, uint8_t data) {
  __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb (uint16_t port) {
  uint8_t data;
  __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
  return data;
}

static inline void outw (uint16_t port, uint16_t data) {
  __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint16_t inw (uint16_t port) {
  uint16_t data;
  __asm__ volatile("inw %1, %0" : "=a"(data) : "Nd"(port));
  return data;
}

static inline void outd (uint16_t port, uint32_t data) {
  __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint32_t ind (uint16_t port) {
  uint32_t data;
  __asm__ volatile("inl %1, %0" : "=a"(data) : "Nd"(port));
  return data;
}

#endif
