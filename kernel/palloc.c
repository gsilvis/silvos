#include "palloc.h"

#include "alloc.h"
#include "memory-map.h"
#include "page.h"
#include "threads.h"

int palloc (void *virt_addr) {
  uint64_t v = (uint64_t)virt_addr;
  if (v & PAGE_4K_MASK) {
    return -2;
  }
  if (v >= LOC_USERZONE_TOP || v < LOC_USERZONE_BOT) {
    return -3;
  }
  return map_new_page(running_tcb->vm_control_block->pt, v, PAGE_MASK__USER);
}

int pfree (void *virt_addr) {
  uint64_t v = (uint64_t)virt_addr;
  if (v & PAGE_4K_MASK) {
    return -2;
  }
  if (v >= LOC_USERZONE_TOP || v < LOC_USERZONE_BOT) {
    return -3;
  }
  return unmap_page(running_tcb->vm_control_block->pt, v);
}
