#ifndef __SILVOS_SYSCALL_H
#define __SILVOS_SYSCALL_H

#include <stdint.h>

#include "syscall-defs.h"

typedef syscall_arg (*syscall_func)(syscall_arg, syscall_arg);

extern syscall_func syscall_defns[NUM_SYSCALLS];
extern uint64_t syscall_defns_len;

#endif

