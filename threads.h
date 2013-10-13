#ifndef __SILVOS_THREADS_H
#define __SILVOS_THREADS_H

enum thread_state {
  TS_NONEXIST, /* Nothing here */
  TS_CREATED,  /* NOTUSING: No execution performed yet */
  TS_BEGUN,    /* Yielded or preempted */
  TS_ENDED,  /* FIXME: Don't think this is necessary? */
};

typedef struct {
  enum thread_state state;
  void *esp;
  void (*task)(void *);
  void *userdata;
} tcb;


/* Takes a pointer to JUST ABOVE THE TOP of the stack and to the BEGINNING of the function */
tcb *thread_create (void *stack, void (*task)(void *), void *userdata);
void schedule (void);
void __attribute__ ((noreturn)) exit (void);
void yield (void);

#endif

