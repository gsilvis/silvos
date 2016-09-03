#include "alloc.h"

/* Current plan: Identity map low memory, and the memory hole.  Identity map
   the kernel at 0x100000.  After that... go wild! */

void *to_alloc;
void *memtop;

extern int _end;

void initialize_allocator (void) {
  unsigned long long tmp = (unsigned long long) &_end;
  if (tmp & 0xFFF) {
    tmp &= 0xFFFFF000;
    tmp += 0x1000;
  }
  to_alloc = (void *) tmp;
}

void *allocate_phys_page (void) {
  if (to_alloc == memtop) {
    return 0x0000000000000000;
  }
  void *returnee = to_alloc;
  to_alloc += 0x1000;
  return returnee;
}
