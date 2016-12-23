#ifndef __SILVOS_MEMORY_MAP_H
#define __SILVOS_MEMORY_MAP_H

#include <stdint.h>

static inline uint64_t phys_to_virt(uint64_t addr) {
  return addr + 0xFFFFFF8000000000;
}

static inline uint64_t virt_to_phys(uint64_t addr) {
  return addr - 0xFFFFFF8000000000;
}

/* First 512G of memory is identity-mapped, kernel-only. */

/* Rest of the lower half is for the userland */

#define LOC_USERZONE_BOT  0x008000000000

#define LOC_TEXT          0x008000000000
#define LOC_USER_STACK    0x7FFFFFFFF000
#define LOC_USER_STACKTOP 0x7FFFFFFFFFF8

#define LOC_USERZONE_TOP  0x800000000000

/* Upper half of memory: currently unused. */

/* Last 512G of memory map to the pagetables.  See page.h for how to find
 * various levels of page table for various locations.
 */


#endif
