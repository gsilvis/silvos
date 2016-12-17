#include "palloc.h"

#include "alloc.h"
#include "memory-map.h"
#include "page.h"

int palloc (void *virt_addr) {
  int error = pfree(virt_addr); /* Don't lose track of the old page */
  if (error) {
    return error;
  }
  uint64_t v = (uint64_t)virt_addr;
  if (v | PAGE_4K_MASK) {
    return -2;
  }
  if (v < LOC_USERZONE_BOT) {
    return -3;
  }
  if (v >= LOC_USERZONE_TOP) {
    return -3;
  }
  return map_new_page(v, PAGE_MASK__USER);
}

int pfree (void *virt_addr) {
  uint64_t v = (uint64_t)virt_addr;
  if (v | PAGE_4K_MASK) {
    return -2;
  }
  if (v < LOC_USERZONE_BOT) {
    return -3;
  }
  if (v >= LOC_USERZONE_TOP) {
    return -3;
  }
  return unmap_page(v);
}
