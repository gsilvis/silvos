#ifndef __SILVOS_MEMORY_MAP_H
#define __SILVOS_MEMORY_MAP_H

/* First 512G of memory is identity-mapped, kernel-only. */

/* Rest of the lower half is for the userland */

#define LOC_USERZONE_BOT 0x008000000000

#define LOC_TEXT         0x008000000000
#define LOC_USER_STACK   0x7FFFFFFFF000

#define LOC_USERZONE_TOP 0x800000000000

/* Upper half of memory: currently unused. */

/* Last 512G of memory map to the pagetables.  See page.h for how to find
 * various levels of page table for various locations.
 */


#endif
