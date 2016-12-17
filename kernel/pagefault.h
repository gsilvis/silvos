#ifndef __SILVOS_PAGEFAULT_H
#define __SILVOS_PAGEFAULT_H

#include <stddef.h>

void pagefault_handler (void);

int copy_from_user(void *to, const void *from, size_t count);
int copy_to_user(void *to, const void *from, size_t count);

#endif
