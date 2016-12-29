#include "pagemap.h"

#include "page-constants.h"

int pm_add_ent (pagemap *p, uint64_t virt, uint64_t phys) {
  if (p->num_entries >= NUMMAPS) {
    return -1;
  }
  p->entries[p->num_entries].virt = virt;
  p->entries[p->num_entries].phys = phys;
  ++p->num_entries;
  return 0;
}

int pm_remove_ent (pagemap *p, uint8_t ix) {
  if (ix >= p->num_entries) {
    return -1;
  }
  p->entries[ix] = p->entries[p->num_entries - 1];
  --p->num_entries;
  return 0;
}

int pm_get_ent (const pagemap *p, uint64_t virt, pagemap_ent *out) {
  for (int i = 0; i < p->num_entries; ++i) {
    if (p->entries[i].virt == virt) {
      if (out) {
        *out = p->entries[i];
      }
      return 0;
    }
  }
  return -1;
}

int pm_get_phys (const pagemap *p, uint64_t virt, uint64_t *phys) {
  pagemap_ent ent;
  int ret = pm_get_ent(p, virt, &ent);
  if (phys && !ret) {
    *phys = ent.phys;
  }

  return ret;
}

int pm_is_mapped (const pagemap *p, uint64_t virt) {
  return pm_get_ent(p, virt, 0) == 0;
}
