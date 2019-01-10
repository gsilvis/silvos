#include "ipc.h"

#include "com.h"
#include "pagefault.h"
#include "threads.h"
#include "util.h"
#include "memory-map.h"

void __attribute__((noreturn)) sendrecv (void) {
  sendrecv_op *usr_op = (sendrecv_op *)running_tcb->saved_registers.rbx;
  ipc_msg send;
  uint64_t sender = running_tcb->thread_id;

  /* Throughout this function, use chained if-else rather than guard clauses to
   * statically verify that every branch ends in an __attribute__((noreturn))
   * function */
  if (copy_from_user(&send, &usr_op->send, sizeof(ipc_msg))) {
    running_tcb->saved_registers.rax = SEND_FAILED;
    return_to_current_thread();
  } else if (send.addr == sender) {
    running_tcb->saved_registers.rax = SEND_FAILED;
    return_to_current_thread();
  } else if (send.addr == 0) {
    /* Message to kernel */
    running_tcb->ipc_state = IPC_RECEIVING;
    schedule();
  } else {
    tcb* recv_thread = get_tcb(send.addr);

    if (!recv_thread) {
      running_tcb->saved_registers.rax = SEND_FAILED;
      return_to_current_thread();
    } else if (recv_thread->ipc_state != IPC_RECEIVING) {
      running_tcb->saved_registers.rax = SEND_FAILED;
      return_to_current_thread();
    } else {
      recv_thread->ipc_state = IPC_RECEIVED;
      recv_thread->inbox = send;
      recv_thread->inbox.addr = sender;
      running_tcb->ipc_state = IPC_RECEIVING;
      switch_thread_to(recv_thread);
    }
  }
}

/* We need to copy_to_user the message to the destination's vm space; the
 * easiest way to do that is to add a hook to 'return_to_userspace' which
 * checks for this case.  Long term, we should instead just pass messages in
 * registers. */
void sendrecv_finish (void) {
  if (running_tcb->ipc_state != IPC_RECEIVED) {
    return;
  }
  sendrecv_op *usr_op = (sendrecv_op *)running_tcb->saved_registers.rbx;
  if (copy_to_user(&usr_op->recv, &running_tcb->inbox, sizeof(ipc_msg))) {
    running_tcb->saved_registers.rax = RECEIVE_FAILED;
  } else {
    running_tcb->saved_registers.rax = MESSAGE_RECEIVED;
  }
}
