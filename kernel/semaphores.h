#ifndef __SILVOS_SEMAPHORES_H
#define __SILVOS_SEMAPHORES_H

#include "syscall-defs.h"

semaphore_id sem_create (void);
int sem_delete (semaphore_id id);
int sem_watch (semaphore_id id);
int sem_unwatch (semaphore_id id);
int sem_set (semaphore_id id);
void __attribute__((noreturn)) sem_wait (void);

#endif
