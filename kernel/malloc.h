#ifndef __SILVOS_MALLOC_H
#define __SILVOS_MALLOC_H

#include <stdint.h>

/* Here's a crappy malloc.  It really sucks for small objects.  Oh well */

void *malloc (uint64_t len);
void free (void *addr);

#endif
