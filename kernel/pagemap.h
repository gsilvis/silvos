#ifndef __SILVOS_PAGE_MAP_H
#define __SILVOS_PAGE_MAP_H
#include <stdint.h>

#define NUMMAPS 32

typedef struct {
  uint64_t virt;
  uint64_t phys;
} pagemap_ent;

typedef struct {
  pagemap_ent entries[NUMMAPS];
  uint8_t num_entries;
} pagemap;

int pm_add_ent (pagemap *p, uint64_t virt, uint64_t phys);
int pm_remove_ent (pagemap *p, uint8_t ix);
int pm_get_ent (const pagemap *p, uint64_t virt, pagemap_ent *out);
int pm_get_phys (const pagemap *p, uint64_t virt, uint64_t *phys);
/* Returns 1 if present, 0 if not present. */
int pm_is_mapped (const pagemap *p, uint64_t virt);

#endif
