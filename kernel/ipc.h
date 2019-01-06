#ifndef __SILVOS_IPC_H
#define __SILVOS_IPC_H

#include <stdint.h>
#include "syscall-defs.h"

/* usr_op is a pointer to userland memory */
sendrecv_status sendrecv (sendrecv_op* usr_op);

#endif
