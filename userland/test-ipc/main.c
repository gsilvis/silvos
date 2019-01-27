#include "userland.h"

void vasily(int addr) {
  sendrecv_op op;
  op.send.addr = addr;
  op.send.r1 = 0;
  op.send.r2 = 0;
  while (1) {
    if (respond(&op)) {
      debug("SEND FAILED");
      return;
    }
    debug("RECEIVED");
    op.send.addr = op.recv.addr;
    op.send.r1 = op.recv.r1 + op.recv.r2;
    op.send.r2 = op.recv.r1 - op.recv.r2;
  }
}

void fedia(int addr) {
  sendrecv_op op;
  op.send.addr = addr;
  op.send.r1 = 0;
  op.send.r2 = 0;
  for (unsigned long long i = 1; i <= 10; ++i) {
    op.send.r1 = i;
    op.send.r2 = 5;
    if (call(&op)) {
      debug("SEND FAILED");
      return;
    }
    if (op.recv.r1 + op.recv.r2 == i * 2 && op.recv.r1 - op.recv.r2 == 10) {
      debug("GOT RIGHT ANSWER");
    } else {
      debug("GOT WRONG ANSWER!!");
    }
  }
}

void main() {
  int parent_tid = get_tid();
  sendrecv_op fork_msgs = {
    .send = {
      .addr = 0,  /* unused */
      .r1 = 0,
      .r2 = 0,
    },
  };
  if (fork_daemon(&fork_msgs)) {
    debug("FORK FAILED");
  }
  if ((int)fork_msgs.recv.addr == parent_tid) {
    /* Received message immediately, so in child. */
    vasily(fork_msgs.recv.addr);
    debug("VASILY EXITS");
  } else {
    /* Didn't, so in parent. */
    fedia(fork_msgs.recv.addr);
    debug("FEDIA EXITS");
  }
}
