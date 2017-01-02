#include "loader.h"

#include "elf.h"
#include "memory-map.h"
#include "page.h"
#include "threads.h"
#include "util.h"

#include <stdint.h>


int elf64_check (uint8_t *elf, uint64_t size) {
  Elf64_Ehdr *elf_header = (Elf64_Ehdr *)elf;
  for (int i = 0; i < 4; i++) {
    if (elf_header->e_ident[i] != ELF_MAGIC[i]) {
      return -1;
    }
  }
  if (elf_header->e_machine != EM_X86_64) {
    return -2;
  }
  if (!elf_header->e_phoff) {
    return -3;
  }
  if (elf_header->e_phoff + elf_header->e_phentsize * elf_header->e_phnum > size) {
    return -4;
  }
  for (size_t i = 0; i < elf_header->e_phnum; i++) {
    Elf64_Phdr *seg = (Elf64_Phdr *)
        &elf[elf_header->e_phoff + i * elf_header->e_phentsize];
    if (seg->p_type != PT_LOAD) {
      continue;
    }
    if (seg->p_offset + seg->p_filesz > size) {
      return -5;
    }
    if (seg->p_vaddr > seg->p_vaddr + seg->p_memsz) {
      return -6;
    }
    if (seg->p_vaddr + seg->p_memsz >= LOC_USERZONE_TOP) {
      return -6;
    }
  }
  return 0;
}

uint64_t elf64_get_entry (uint8_t *elf) {
  Elf64_Ehdr *elf_header = (Elf64_Ehdr *)elf;
  return (uint64_t)(elf_header->e_entry);
}

void elf64_load (uint8_t *elf) {
  Elf64_Ehdr *elf_header = (Elf64_Ehdr *)elf;
  for (size_t i = 0; i < elf_header->e_phnum; i++) {
    Elf64_Phdr *seg = (Elf64_Phdr *)
        &elf[elf_header->e_phoff + i * elf_header->e_phentsize];
    if (seg->p_type != PT_LOAD) {
      continue;
    }
    if (!seg->p_memsz) {
      continue;
    }
    if (seg->p_filesz > seg->p_memsz) {
      continue; /* Why would you do that */
    }
    for (uint64_t page = PAGE_4K_ALIGN(seg->p_vaddr);
         page < seg->p_vaddr + seg->p_memsz;
         page += PAGE_4K_SIZE) {
      uint64_t flags = PAGE_MASK_PRIV;
      if (!(seg->p_flags & PF_X)) {
        flags |= PAGE_MASK_NX;
      }
      if (seg->p_flags & PF_W) {
        flags |= PAGE_MASK_WRITE;
      }
      /* I don't believe there's anything useful to do with the PF_R flag */
      map_new_page(running_tcb->pt, page, PAGE_MASK__USER);
    }
    memcpy((void *)seg->p_vaddr, (void *)&elf[seg->p_offset], seg->p_filesz);
    memset((void *)seg->p_vaddr + seg->p_filesz, 0, seg->p_memsz - seg->p_filesz);
  }
}
