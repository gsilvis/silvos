#include "alloc.h"

#include "util.h"

#include <stdint.h>

extern int _end;

/* Buddy allocator, inspired by the linux kernel's version.  Can allocate pages
 * of sizes 4K - 1G */

typedef uint8_t block_alloc_array[0x8000];

block_alloc_array *bit_arrays;

typedef struct freelist_ {
  struct freelist_ *next;
  struct freelist_ *prev;
} freelist;

freelist free_blocks[19];

freelist *get_freelist (int bsize, uint64_t index, uint64_t gig_offset) {
  index = index << (30 - bsize);
  index += 0x40000000 * gig_offset;
  return (freelist *)index;
}

uint64_t get_index (int bsize, freelist *ptr) {
  return (uint64_t)ptr >> (30 - bsize);
}

void free_block (int bsize, uint64_t index) {
  uint64_t per_gig = 0x1 << bsize;
  uint64_t gig_offset = index / per_gig;
  index = index % per_gig;
  for (; bsize > 0; bsize--, index /= 2) {
    if (bit_array_get((uint8_t *)&bit_arrays[gig_offset], per_gig + index/2)) {
      bit_array_set((uint8_t *)&bit_arrays[gig_offset], per_gig + index/2, 0);
      freelist *new = get_freelist(bsize, index, gig_offset);
      /* insert into free list */
      new->prev = &free_blocks[bsize];
      new->next = free_blocks[bsize].next;
      free_blocks[bsize].next->prev = new;
      free_blocks[bsize].next = new;
      return;
    }
    /* remove buddy from free list */
    uint64_t buddy = index ^ 0x1;
    freelist *old = get_freelist(bsize, buddy, gig_offset);
    old->prev->next = old->next;
    old->next->prev = old->prev;
    /* remove suspicious information */
    old->prev = NULL;
    old->next = NULL;
  }
  /* put entire 1G block into free list */
  freelist *new = get_freelist(0, 0, gig_offset);
  new->prev = &free_blocks[0];
  new->next = free_blocks[0].next;
  free_blocks[0].next->prev = new;
  free_blocks[0].next = new;
}

void *alloc_block (int bsize) {
  int b = bsize;
  for (; b >= 0; b--) {
    if (free_blocks[b].next != &free_blocks[b]) {
      /* Found some free memory */
      break;
    }
  }
  if (b == -1) {
    /* Didn't find any free memory */
    return NULL;
  }
  freelist *to_return = free_blocks[b].next;
  /* unlink it */
  to_return->prev->next = to_return->next;
  to_return->next->prev = to_return->prev;
  /* mark it used */
  uint64_t index = get_index(b, to_return);
  uint64_t gig_offset = index / (0x1 << b);
  index = index % (0x1 << b);
  bit_array_set((uint8_t *)&bit_arrays[gig_offset], (0x1 << b) + index/2, 1);
  b++; index *= 2;
  for (; b <= bsize; b++, index *= 2) {
    /* make buddy blocks free */
    freelist *to_add = get_freelist(b, index + 1, gig_offset);
    to_add->prev = &free_blocks[b];
    to_add->next = free_blocks[b].next;
    free_blocks[b].next->prev = to_add;
    free_blocks[b].next = to_add;
    bit_array_set((uint8_t *)&bit_arrays[gig_offset], (0x1 << b) + index/2, 0);
  }
  /* remove suspicious information */
  to_return->prev = NULL;
  to_return->next = NULL;
  return (void *)to_return;
}

void initialize_allocator (void) {
  /* Allocate bit-arrays, initialize as "all memory used" */
  uint64_t num_gigs = (memtop - 1) / 0x40000000 + 1;
  uint64_t curr = (uint64_t) &_end;
  if (curr & 0x7FFF) {
    curr &= 0xFFFFFFFFFFFF8000;
    curr += 0x8000;
  }
  bit_arrays = (block_alloc_array *)curr;
  curr += num_gigs * 0x8000;
  memset(bit_arrays, 0xFF, num_gigs * 0x8000);

  /* Initialize freelists as empty */
  for (int i = 0; i < 19; i++) {
    free_blocks[i].next = &free_blocks[i];
    free_blocks[i].prev = &free_blocks[i];
  }

  /* Free all memory between bit-arrays and memtop */
  int bsize = 18; /* 0x1000 */
  uint64_t index = curr / 0x1000;
  for (; bsize >= 0; bsize--, index /= 2) {
    if (index % 2 == 0) {
      continue;
    }
    uint64_t blksize = (0x1 << (30-bsize));
    if (curr + blksize > memtop) {
      break;
    }
    free_block(bsize, index);
    index++;
    curr += blksize;
  }
  for (; bsize < 19; bsize++, index *= 2) {
    uint64_t blksize = (0x1 << (30-bsize));
    if (curr + blksize > memtop) {
      continue;
    }
    free_block(bsize, index);
    index++;
    curr += blksize;
  }
}

void *allocate_phys_page (void) {
  return alloc_block(18);
}

void free_phys_page (void *page) {
  uint64_t index = (uint64_t)page;
  index = index >> 12;
  free_block(18, index);
}
