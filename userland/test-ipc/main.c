#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void vasily() {
  sendrecv_op op;
  op.send.addr = 0;
  while (1) {
    switch (sendrecv(&op)) {
      case SEND_FAILED:
        DEBUG("SEND FAILED");
        return;
      case RECEIVE_FAILED:
        DEBUG("RECEIVE FAILED");
        return;
      case MESSAGE_RECEIVED:
        DEBUG("RECEIVED");
        op.send.addr = op.recv.addr;
        op.send.r1 = op.recv.r1 + op.recv.r2;
        op.send.r2 = op.recv.r1 - op.recv.r2;
        break;
    }
  }
}

void fedia(int addr) {
  sendrecv_op op;
  op.send.addr = addr;
  for (unsigned long long i = 1; i <= 10; ++i) {
    op.send.r1 = i;
    op.send.r2 = 5;
    switch (sendrecv(&op)) {
      case SEND_FAILED:
        DEBUG("SEND_FAILED");
        return;
      case RECEIVE_FAILED:
        DEBUG("RECEIVE FAILED");
        return;
      case MESSAGE_RECEIVED:
        if (op.recv.r1 + op.recv.r2 == i * 2 && op.recv.r1 - op.recv.r2 == 10) {
          DEBUG("GOT RIGHT ANSWER");
        } else {
          DEBUG("GOT WRONG ANSWER!!");
        }
        break;
    }
  }
}

void main() {
  int child_pid = fork();
  if (child_pid == 0) {
    vasily();
    /* This should not execute */
    DEBUG("VASILY EXITS");
  } else {
    yield();
    fedia(child_pid);
    DEBUG("FEDIA EXITS");
  }
}
