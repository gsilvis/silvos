#ifndef __SILVOS_PAGEFAULT_H
#define __SILVOS_PAGEFAULT_H

#include <stddef.h>
#include <stdint.h>

int copy_from_user (void *to, const void *from, size_t count);
int copy_to_user (void *to, const void *from, size_t count);
int copy_string_from_user (void *to, const void *from, size_t max);

#endif
