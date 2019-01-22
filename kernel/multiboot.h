#ifndef __SILVOS_MULTIBOOT_H
#define __SILVOS_MULTIBOOT_H

#include <stdint.h>

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t string;
  uint32_t unused;
} multiboot_module;

void spawn_multiboot_modules (multiboot_module *mod_list, uint32_t length);

/* Find the TCB entry corresponding to the initial process spawned from a
 * specific multiboot module. The pointer comes from userspace. */
int find_module (const char *name);

#endif
