#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

int thread_create (int *stack, void (*task)(void *), void *userdata);
void yield (void);

#endif
