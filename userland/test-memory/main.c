#include "userland.h"

static inline unsigned char* getpage(unsigned int i) {
  return (unsigned char*) ((1 << i) * 4096L);
}

static inline unsigned char getent(unsigned int i, unsigned int j, int pid) {
  return ((i * 37) ^ j ^ pid) & 0xff;
}

void main() {
  debug("Allocating");
  const unsigned int num_pages = 7;
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    int ret = palloc(my_page);
    if (ret) {
      debug("Failed to allocate page!");
      exit();
    }
  }

  debug("Forking");
  int pid = fork();
  if (pid < 0) {
    debug("Failed to fork!");
    exit();
  }
  if (pid == 0) {
    debug("Child");
  } else {
    debug("Parent");
  }
  yield();

  debug("Writing");
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      my_page[j] = getent(i, j, pid);
    }
  }
  yield();

  debug("Reading");
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      unsigned char ent = my_page[j];
      if (ent != getent(i, j, pid)) {
        debug("Values differed (before freeing a page)");
      }
    }
  }
  yield();

  debug("Freeing");
  unsigned char* freed_page = getpage(2);
  if (pfree(freed_page)) {
    debug("Failed to free a page");
    exit();
  }
  yield();

  debug("Reading");
  for (unsigned int i = 0; i < num_pages; ++i) {
    // Skip the third page because it is freed.
    if (i == 2) {
      continue;
    }
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      unsigned char ent = my_page[j];
      if (ent != getent(i, j, pid)) {
        debug("Values differed (after freeing a page)");
      }
    }
  }
  yield();

  debug("Page faulting");
  getpage(2)[0] = 42;
  debug("We should not get here after triggering a page fault.");
}
