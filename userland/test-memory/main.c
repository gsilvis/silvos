#include "userland.h"
#define DEBUG(str) debug(str, sizeof(str))

inline unsigned char* getpage(unsigned int i) {
  return (unsigned char*) ((1 << i) * 4096L);
}

inline unsigned char getent(unsigned int i, unsigned int j) {
  return ((i * 37) ^ j) & 0xff;
}

void main() {
  // TODO: Use our PID or a rand() to seed the values stored in our allocated
  // pages, and run two of these at the same time (after clone() or fork()).
  DEBUG("Writing");
  const unsigned int num_pages = 7;
  for (unsigned int i = 0; i < num_pages; ++i) {
    // Use unsigned char to prevent problems when extending to unsigned longs.
    unsigned char* my_page = getpage(i);
    int ret = palloc(my_page);
    if (ret) {
      DEBUG("Failed to allocate page!");
      exit();
    }

    for (unsigned int j = 0; j < 4096; ++j) {
      my_page[j] = getent(i, j);
    }
    yield();
  }

  DEBUG("Reading");
  for (unsigned int i = 0; i < num_pages; ++i) {
    unsigned char* my_page = getpage(i);
    for (unsigned int j = 0; j < 4096; ++j) {
      unsigned char ent = my_page[j];
      if (ent != getent(i, j)) {
        DEBUG("Values differed (before freeing a page)");
      }
    }
    yield();
  }

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
      if (ent != getent(i, j)) {
        DEBUG("Values differed (after freeing a page)");
      }
    }
    yield();
  }

  DEBUG("Page faulting");
  getpage(2)[0] = 42;
  DEBUG("We should not get here after triggering a page fault.");
}
