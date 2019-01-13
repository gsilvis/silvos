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

void __attribute__((noreturn)) sendrecv (void) {
  ipc_msg send;
  send.addr = running_tcb->saved_registers.rbx;
  send.r1 = running_tcb->saved_registers.rcx;
  send.r2 = running_tcb->saved_registers.rdx;
  uint64_t sender = running_tcb->thread_id;

  /* Throughout this function, use chained if-else rather than guard clauses to
   * statically verify that every branch ends in an __attribute__((noreturn))
   * function */
  if (send.addr == sender) {
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
      recv_thread->ipc_state = IPC_NOT_RECEIVING;
      recv_thread->saved_registers.rax = MESSAGE_RECEIVED;
      recv_thread->saved_registers.rbx = sender;
      recv_thread->saved_registers.rcx = send.r1;
      recv_thread->saved_registers.rdx = send.r2;
      running_tcb->ipc_state = IPC_RECEIVING;
      switch_thread_to(recv_thread);
    }
  }
}
