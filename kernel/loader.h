#ifndef __SILVOS_LOADER_H
#define __SILVOS_LOADER_H

#include <stdint.h>

int elf64_check (uint8_t *elf, uint64_t size);
uint64_t elf64_get_entry (uint8_t *elf);
void elf64_load (uint8_t *elf);

#endif
