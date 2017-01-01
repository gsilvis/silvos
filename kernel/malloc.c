#include "malloc.h"

#include "alloc.h"

#include <stdint.h>
#include <stddef.h>

/* Here's a crappy malloc.  It really sucks for small objects.  Oh well */

void *malloc (uint64_t len) {
  if (len == 0) {
    return NULL;
  }
  len += 8;
  uint64_t *returnee = NULL;
  for (int8_t bsize = 18; bsize >= 0; bsize--) {
    uint64_t size = 1 << (30 - bsize);
    if (size >= len) {
      returnee = (uint64_t *)alloc_block(bsize);
      returnee[0] = bsize;
      returnee = &returnee[1];
      break;
    }
  }
  if (!returnee) {
    return NULL; /* too big! */
  }
  return (void *)returnee;
}

void free (void *addr) {
  if (addr == NULL) {
    return;
  }
  uint64_t *storage = (uint64_t *)addr;
  storage = &storage[-1];
  int8_t bsize = storage[0];
  free_block(bsize, get_index(bsize, (void *)&storage));
  return;
}
