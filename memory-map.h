#ifndef __SILVOS_MEMORY_MAP_H
#define __SILVOS_MEMORY_MAP_H


/* 0x000000000000 - 0x0000001FFFFF
 *
 * First 2M page has a single page directory, used by all threads.  All
 * kernel data structures must be mapped here.
 */

/* Null page:          0x000000000000  -  0x000000000FFF */

#define LOC_FP_BUF     0x0000000E0000  /* 0x0000000EFFFF  (16 pages) */
#define LOC_IDT        0x0000000F0000  /* 0x0000000F0FFF  (1 page)   */
#define LOC_IST1_STACK 0x0000000FC000  /* 0x0000000FCFFF  (1 page)   */
#define LOC_TEMP_PT    0x0000000FD000  /* 0x0000000FDFFF  (1 page)   */
#define LOC_TEMP_PT2   0x0000000FE000  /* 0x0000000FEFFF  (1 page)   */
#define LOC_IDLE_STACK 0x0000000FF000  /* 0x0000000FFFFF  (1 page)   */

/* Kernel text:        0x000000100000  -  0x0000001FFFFF */

#define LOC_USERZONE_BOT 0x000000200000

/* 0x000040000000 - 0x7FFFFFFFFFFF
 *
 * Lower half of memory: contains thread-specific information
 */

#define LOC_TEXT        0x000040000000
#define LOC_USER_STACK  0x0000401FE000
#define LOC_KERN_STACK  0x0000401FF000

/* 0x800000000000 - 0xFF7FFFFFFFFF
 *
 * Upper half of memory: currently unused.
 */

#define LOC_USERZONE_TOP 0xFF8000000000

/* 0xFF8000000000 - 0xFFFFFFFFFFFF
 *
 * Last 512G of memory map to the pagetables.  See page.h for how to find
 * various levels of page table for various locations.
 */


#endif
