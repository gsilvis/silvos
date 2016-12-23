#ifndef __SILVOS_MEMORY_MAP_H
#define __SILVOS_MEMORY_MAP_H

#include <stdint.h>

static inline uint64_t phys_to_virt(uint64_t addr) {
  return addr + 0xFFFFFF8000000000;
}

static inline uint64_t virt_to_phys(uint64_t addr) {
  return addr - 0xFFFFFF8000000000;
}

/* Lower half of memory is for userland */

#define LOC_USERZONE_BOT  0x000000000000

#define LOC_TEXT          0x000000000000
#define LOC_USER_STACK    0x7FFFFFFFF000
#define LOC_USER_STACKTOP 0x7FFFFFFFFFF8

#define LOC_USERZONE_TOP  0x800000000000

/* Upper half of memory is for the kernel. */

/* Second-to-last 512G of memory map to the pagetables.  See page.h for how to
 * find various levels of page table for various locations.
 *
 * PAGE_TABLE_START  0xFF0000000000
 * PAGE_TABLE_END    0xFF7FFFFFFFFF
 */

/* Last 512G of memory are for kernel use.  There are two separate maps here:
 * the last 2G maps to the first 2G of memory, for kernel symbols.  The rest
 * also maps to low memory, for allocations inside the kernel.
 *
 * ALLOCATABLE_MEMORY_START  0xFF8000000000    (0x000000000000)
 * ALLOCATABLE_MEMORY_END    0xFFFF7FFFFFFF    (0x00F7FFFFFFFF)
 *
 * KERNEL_SYMBOLS_START      0xFFFF80000000    (0x000000000000)
 * KERNEL_SYMBOLS_END        0xFFFFFFFFFFFF    (0x00007FFFFFFF)
 */


#endif
