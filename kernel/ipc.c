#include "ipc.h"

#include "com.h"
#include "pagefault.h"
#include "threads.h"
#include "util.h"
#include "memory-map.h"

sendrecv_status sendrecv (sendrecv_op* usr_op) {
  ipc_msg send;
  if (copy_from_user(&send, &usr_op->send, sizeof(ipc_msg))) return SEND_FAILED;

  uint64_t sender = running_tcb->thread_id;
  uint64_t addr = send.addr;

  if (addr == sender) return SEND_FAILED;

  if (addr != 0) {
    tcb* recv_thread = get_tcb(addr);
    if (!recv_thread) return SEND_FAILED;
    if (recv_thread->ipc_state != IPC_RECEIVING) return SEND_FAILED;
    recv_thread->ipc_state = IPC_RECEIVED;
    recv_thread->inbox = send;
    recv_thread->inbox.addr = sender;
    /* Make sure we wake up the receiver immediately */
    promote(recv_thread);
  }

  running_tcb->ipc_state = IPC_RECEIVING;
  schedule();
  running_tcb->ipc_state = IPC_NOT_RECEIVING;

  /* TODO: This is bad. Just put the messages in registers / use sysenter */
  if (copy_to_user(&usr_op->recv, &running_tcb->inbox, sizeof(ipc_msg))) {
    return RECEIVE_FAILED;
  }
  return MESSAGE_RECEIVED;
}
