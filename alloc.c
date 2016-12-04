#include "alloc.h"

#include <stdint.h>

void *to_alloc;
void *memtop;

extern int _end;

struct free_list {
  struct free_list *next;
} *free_list_start;

void initialize_allocator (void) {
  uint64_t tmp = (uint64_t) &_end;
  if (tmp & 0xFFF) {
    tmp &= 0xFFFFF000;
    tmp += 0x1000;
  }
  to_alloc = (void *) tmp;
  free_list_start = 0x0000000000000000;
}

void *allocate_phys_page (void) {
  if (free_list_start) {
    struct free_list *tmp = free_list_start->next;
    free_list_start->next = (struct free_list *) 0x0000000000000000;
    void *returnee = free_list_start;
    free_list_start = tmp;
    return returnee;
  }
  if (to_alloc == memtop) {
    return 0x0000000000000000;
  }
  void *returnee = to_alloc;
  to_alloc += 0x1000;
  return returnee;
}

void free_phys_page (void *page) {
  struct free_list *new_head = (struct free_list *)page;
  new_head->next = free_list_start;
  free_list_start = new_head;
}
