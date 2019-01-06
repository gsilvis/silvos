#include "pagefault.h"

#include "com.h"
#include "memory-map.h"
#include "threads.h"
#include "util.h"

#include <stddef.h>
#include <stdint.h>

int check_addr (const void *low, const void *high) {
  if ((uint64_t)low > (uint64_t)high) {
    return -1;
  }
  if ((uint64_t)high > LOC_USERZONE_TOP) {
    return -1;
  }
  return 0;
}

jmp_buf for_copying;
int is_copying;

void pagefault_handler_copy (void) {
  if (is_copying) {
    longjmp(for_copying, -1);
  }
}

void __attribute__((noreturn)) pagefault_handler_user (uint64_t addr) {
  if (running_tcb) {
    com_printf("Kernel page fault at %p, running thread is 0x%02X.\n", (void *)addr, running_tcb->thread_id);
    thread_exit();
  } else {
    com_printf("Kernel page fault at %p, no running thread! Panic!\n", (void *)addr);
    panic("Page fault with no running thread!");
  }
}


int copy_from_user(void *to, const void *from, size_t count) {
  const char *f = from;
  if (check_addr(f, f+count)) {
    return -1;
  }
  is_copying = 1;
  int res = setjmp(for_copying);
  if (res == 0) {
    memcpy(to, from, count);
  }
  is_copying = 0;
  return res;
}


int copy_to_user(void *to, const void *from, size_t count) {
  char *t = to;
  if (check_addr(t, t+count)) {
    return -1;
  }
  is_copying = 1;
  int res = setjmp(for_copying);
  if (res == 0) {
    memcpy(to, from, count);
  }
  is_copying = 0;
  return res;
}
