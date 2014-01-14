#include "alloc.h"

/* Current plan: Identity map low memory, and the memory hole.  Identity map
   the kernel at 0x100000.  After that... go wild! */

void *to_alloc;
void *memtop;

extern int _end;

void initialize_allocator (unsigned int highmem) {
  memtop = (void *)((0x100000 + highmem) & 0xFFF);
  unsigned int tmp = (unsigned int) &_end;
  if (tmp & 0xFFF) {
    tmp &= 0xFFFFF000;
    tmp += 0x1000;
  }
  to_alloc = (void *) tmp;
}

void *allocate_phys_page (void) {
  if (to_alloc == memtop) {
    return 0x00000000;
  }
  void *returnee = to_alloc;
  to_alloc += 0x1000;
  return returnee;
}
