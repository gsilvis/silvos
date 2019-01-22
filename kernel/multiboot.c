#include "multiboot.h"

#include "com.h"
#include "malloc.h"
#include "memory-map.h"
#include "pagefault.h"
#include "threads.h"
#include "util.h"

typedef struct {
  uint8_t tid;
  const char *name;
} loaded_module;

static loaded_module *modules;
static uint32_t num_modules;

static void *mboot_to_addr (uint32_t addr) {
  return (void *)phys_to_virt((uint64_t)addr);
}

static const char *split_fname (const char *full_path) {
  /* the module name should be 'foo.bin', not './userland/foo.bin'. */
  const char *current_guess = full_path;
  const char *curs = full_path;
  while (*curs) {
    if (*curs == '/') {
      current_guess = curs + 1;
    }
    ++curs;
  }
  return current_guess;
}

int find_module (const char *name) {
  char buf[4096];
  if (copy_string_from_user(buf, name, sizeof(buf))) return -1;
  buf[4095] = 0;  /* in case of truncation */
  size_t n = strlen(name);
  for (uint32_t i = 0; i < num_modules; ++i) {
    if (strncmp(name, modules[i].name, n)) continue;
    return modules[i].tid;
  }
  return -1;
}

void spawn_multiboot_modules (multiboot_module *mod_list, uint32_t length) {
  num_modules = length;
  modules = malloc(sizeof(loaded_module) * num_modules);
  for (uint32_t i = 0; i < num_modules; ++i) {
    modules[i].name = split_fname(mboot_to_addr(mod_list[i].string));
    modules[i].tid = user_thread_create(mboot_to_addr(mod_list[i].start),
                                        mod_list[i].end - mod_list[i].start);
  }
}
