#include "userland.h"
#define DEBUG(str) debug(str, sizeof(str))

inline unsigned char* getpage(unsigned int i) {
  return (unsigned char*) ((1 << i) * 4096L);
}

inline unsigned char getent(unsigned int i, unsigned int j, int pid) {
  return ((i * 37) ^ j ^ pid) & 0xff;
}

void main() {
  DEBUG("Allocating");
  const unsigned int num_pages = 7;
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    int ret = palloc(my_page);
    if (ret) {
      DEBUG("Failed to allocate page!");
      exit();
    }
  }

  DEBUG("Forking");
  int pid = fork();
  if (pid < 0) {
    DEBUG("Failed to fork!");
    exit();
  }
  if (pid == 0) {
    DEBUG("Child");
  } else {
    DEBUG("Parent");
  }
  yield();

  DEBUG("Writing");
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      my_page[j] = getent(i, j, pid);
    }
  }
  yield();

  DEBUG("Reading");
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      unsigned char ent = my_page[j];
      if (ent != getent(i, j, pid)) {
        DEBUG("Values differed (before freeing a page)");
      }
    }
  }
  yield();

  DEBUG("Freeing");
  unsigned char* freed_page = getpage(2);
  if (pfree(freed_page)) {
    DEBUG("Failed to free a page");
    exit();
  }
  yield();

  DEBUG("Reading");
  for (unsigned int i = 0; i < num_pages; ++i) {
    // Skip the third page because it is freed.
    if (i == 2) {
      continue;
    }
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      unsigned char ent = my_page[j];
      if (ent != getent(i, j, pid)) {
        DEBUG("Values differed (after freeing a page)");
      }
    }
  }
  yield();

  DEBUG("Page faulting");
  getpage(2)[0] = 42;
  DEBUG("We should not get here after triggering a page fault.");
}
