#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

int user_thread_create (unsigned char *text, unsigned int length);
void schedule_helper (void);
void thread_exit (void);
void thread_start (void);
void schedule (void);

#endif
