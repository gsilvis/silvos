#include "userland.h"

/* 256 MB */
#define TOTAL_MEM_SIZE (256 * 1024 * 1024)
/* 2 MB */
#define MEM_PER_THREAD (2 * 1024 * 1024)
/* Base of zone to allocate at */
#define MEMORY_BASE (unsigned long)(0x10000000)

#define PAGE_SIZE 0x1000

void main() {
  for (int i = 0; i < TOTAL_MEM_SIZE/MEM_PER_THREAD; i++) {
    int f = fork();
    if (f) {
       nanosleep(1000);
       continue;
    }
    debug("Thread Start");
    for (int j = 0; j < (MEM_PER_THREAD / PAGE_SIZE); j++) {
      if (palloc((char *)(MEMORY_BASE + j * PAGE_SIZE))) {
        debug("ALLOC FAIL");
        exit();
      }
    }
    debug("Alloc Done");
    exit();
  }
  debug("Master DONE");
}
