#ifndef __SILVOS_IPC_H
#define __SILVOS_IPC_H

typedef struct {
  unsigned long long addr;
  unsigned long long r1;
  unsigned long long r2;
} ipc_msg;

/* Send the given message (and therefore don't return) if possible; otherwise
 * return.  Example usage:
 *
 *   call_if_possible(my_message);
 *   thread_exit();
 */
void call_if_possible (ipc_msg msg);

/* Implementations of user-facing 'call' and 'respond' syscalls. */
void __attribute__((noreturn)) call (void);
void __attribute__((noreturn)) respond (void);

#endif
