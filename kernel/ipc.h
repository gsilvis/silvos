#ifndef __SILVOS_IPC_H
#define __SILVOS_IPC_H

#include "syscall-defs.h"

void __attribute__((noreturn)) sendrecv (void);
void sendrecv_finish (void);

#endif
