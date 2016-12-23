#include "pagefault.h"

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

void pagefault_handler (void) {
  if (is_copying) {
    longjmp(for_copying, -1);
  } else {
    thread_exit();
    schedule();
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
