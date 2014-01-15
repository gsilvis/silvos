#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

int user_thread_create (void (*task)(void));
void schedule (void);

#endif
