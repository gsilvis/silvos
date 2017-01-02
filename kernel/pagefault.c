#include "pagefault.h"

#include "com.h"
#include "util.h"
#include "memory-map.h"
#include "threads.h"

#include <stdint.h>
#include <stddef.h>

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

void pagefault_handler (uint64_t addr) {
  if (is_copying) {
    longjmp(for_copying, -1);
  }
  if (!try_remap_cow(running_tcb->pt, PAGE_4K_ALIGN(addr))) {
    return;
  }
  com_puts("Kernel: ");
  com_put_tid();
  com_puts(" page fault at ");
  char pf_addr[] = "0x0000000000000000";
  const char hex[] = "0123456789ABCDEF";
  for (int i = 0; i < 16; ++i) {
    pf_addr[sizeof(pf_addr) - 2 - i] = hex[0x0F & (addr >> (4 * i))];
  }
  com_puts(pf_addr);
  com_putch('\n');
  thread_exit();
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
