#ifndef __SILVOS_PAGE_H
#define __SILVOS_PAGE_H

#include <stdint.h>

#define PAGE_MASK_PRESENT 0x0000000000000001
#define PAGE_MASK_WRITE   0x0000000000000002
#define PAGE_MASK_PRIV    0x0000000000000004
#define PAGE_MASK_WTCACHE 0x0000000000000008
#define PAGE_MASK_NOCACHE 0x0000000000000010
#define PAGE_MASK_ACCESS  0x0000000000000020
#define PAGE_MASK_SIZE    0x0000000000000080

#define PAGE_MASK__KERNEL (PAGE_MASK_PRESENT | PAGE_MASK_WRITE)
#define PAGE_MASK__USER (PAGE_MASK_PRESENT | PAGE_MASK_WRITE | PAGE_MASK_PRIV)
#define PAGE_MASK__FAKE 0x0000000000000000

static const uint64_t PAGE_4K_MASK = (1L << 12) - 1;
static const uint64_t PAGE_2M_MASK = (1L << 21) - 1;
static const uint64_t PAGE_1G_MASK = (1L << 30) - 1;
static const uint64_t PAGE_HT_MASK = (1L << 39) - 1;
static const uint64_t PAGE_VM_MASK = (1L << 48) - 1;

static const uint64_t PAGE_4K_SIZE = 1L << 12;
static const uint64_t PAGE_2M_SIZE = 1L << 21;
static const uint64_t PAGE_1G_SIZE = 1L << 30;
static const uint64_t PAGE_HT_SIZE = 1L << 39;
static const uint64_t PAGE_VM_SIZE = 1L << 48;

static inline uint64_t PAGE_4K_OF(uint64_t mem) { return 0x1FF & (mem >> 12); }
static inline uint64_t PAGE_2M_OF(uint64_t mem) { return 0x1FF & (mem >> 21); }
static inline uint64_t PAGE_1G_OF(uint64_t mem) { return 0x1FF & (mem >> 30); }
static inline uint64_t PAGE_HT_OF(uint64_t mem) { return 0x1FF & (mem >> 39); }

static inline uint64_t PAGE_4K_ALIGN(uint64_t mem) { return mem & ~PAGE_4K_MASK; }
static inline uint64_t PAGE_2M_ALIGN(uint64_t mem) { return mem & ~PAGE_2M_MASK; }
static inline uint64_t PAGE_1G_ALIGN(uint64_t mem) { return mem & ~PAGE_1G_MASK; }
static inline uint64_t PAGE_HT_ALIGN(uint64_t mem) { return mem & ~PAGE_HT_MASK; }

static const uint64_t PAGE_PT_NUM_ENTRIES = 512;
static const uint64_t PAGE_PT_SELF_MAP = 510;

static inline uint64_t PAGE_MAKE_CANONICAL_ADDR(uint64_t mem) {
  uint64_t lead = ((1L << 47) & mem) ? ~PAGE_VM_MASK : 0;
  return lead | mem;
}

static inline uint64_t PAGE_VIRT_PT_OF(uint64_t mem) {
  uint64_t lower = (mem & PAGE_VM_MASK & ~PAGE_2M_MASK) >> 9;
  uint64_t upper = (PAGE_PT_SELF_MAP << 39);
  return PAGE_MAKE_CANONICAL_ADDR(lower | upper);
}

static inline uint64_t PAGE_VIRT_PD_OF(uint64_t mem) {
  uint64_t lower = (mem & PAGE_VM_MASK & ~PAGE_1G_MASK) >> 18;
  uint64_t upper = (PAGE_PT_SELF_MAP << 39)
                 | (PAGE_PT_SELF_MAP << 30);
  return PAGE_MAKE_CANONICAL_ADDR(lower | upper);
}

static inline uint64_t PAGE_VIRT_PDPT_OF(uint64_t mem) {
  uint64_t lower = (mem & PAGE_VM_MASK & ~PAGE_HT_MASK) >> 27;
  uint64_t upper = (PAGE_PT_SELF_MAP << 39)
                 | (PAGE_PT_SELF_MAP << 30)
                 | (PAGE_PT_SELF_MAP << 21);
  return PAGE_MAKE_CANONICAL_ADDR(lower | upper);
}

static inline uint64_t PAGE_VIRT_PML4_OF(uint64_t mem) {
  uint64_t lower = (mem & PAGE_VM_MASK & ~PAGE_VM_MASK) >> 36;
  uint64_t upper = (PAGE_PT_SELF_MAP << 39)
                 | (PAGE_PT_SELF_MAP << 30)
                 | (PAGE_PT_SELF_MAP << 21)
                 | (PAGE_PT_SELF_MAP << 12);
  return PAGE_MAKE_CANONICAL_ADDR(lower | upper);
}

typedef uint64_t *pagetable;

void insert_pt (pagetable pt);
pagetable get_current_pt (void);
pagetable initial_pt (void);
pagetable new_pt (void);
uint64_t virt_to_phys (uint64_t virt);
int map_page (uint64_t phys, uint64_t virt, unsigned int mode);
int unmap_page (uint64_t virt);
int map_new_page (uint64_t virt, unsigned int mode);
int remap_page (uint64_t virt_from, uint64_t virt_to, unsigned int mode);

#endif
