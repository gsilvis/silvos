#include "ipc.h"

#include "com.h"
#include "syscall-defs.h"
#include "threads.h"
#include "util.h"

typedef struct {
  unsigned long long addr;
  unsigned long long r1;
  unsigned long long r2;
} ipc_msg;

static inline ipc_msg get_msg (void) {
  ipc_msg send = {
    .addr = running_tcb->saved_registers.rbx,
    .r1 = running_tcb->saved_registers.rcx,
    .r2 = running_tcb->saved_registers.rdx,
  };
  return send;
}

static void __attribute__((noreturn)) do_send (ipc_msg send, tcb *recv_tcb) {
  recv_tcb->ipc_state = IPC_NOT_RECEIVING;
  recv_tcb->saved_registers.rax = MESSAGE_RECEIVED;
  recv_tcb->saved_registers.rbx = running_tcb->thread_id;
  recv_tcb->saved_registers.rcx = send.r1;
  recv_tcb->saved_registers.rdx = send.r2;
  switch_thread_to(recv_tcb);
}

void __attribute__((noreturn)) call (void) {
  ipc_msg send = get_msg();
  tcb* recv_thread = get_tcb(send.addr);

  if (recv_thread == 0 ||
      recv_thread == running_tcb ||
      recv_thread->ipc_state != IPC_DAEMON) {
    running_tcb->saved_registers.rax = SEND_FAILED;
    return_to_current_thread();
  }

  running_tcb->ipc_state = IPC_CALLING;
  running_tcb->callee = send.addr;
  do_send(send, recv_thread);
}

void __attribute__((noreturn)) respond (void) {
  ipc_msg resp = get_msg();
  tcb* recv_thread = get_tcb(resp.addr);

  if (resp.addr == 0) {
    /* Request to daemonize.  If our parent is waiting, switch back to them;
     * otherwise, invoke the scheduler. */
    running_tcb->ipc_state = IPC_DAEMON;
    tcb *maybe_parent = running_tcb->parent;
    running_tcb->parent = 0;
    switch_thread_to(maybe_parent);
  } else if (recv_thread == 0 ||
             recv_thread->ipc_state != IPC_CALLING ||
             recv_thread->callee != running_tcb->thread_id) {
    running_tcb->saved_registers.rax = SEND_FAILED;
    return_to_current_thread();
  }

  running_tcb->ipc_state = IPC_DAEMON;
  do_send(resp, recv_thread);
}
