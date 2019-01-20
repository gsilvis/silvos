#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void vasily() {
  sendrecv_op op;
  op.send.addr = 0;
  op.send.r1 = 0;
  op.send.r2 = 0;
  while (1) {
    if (respond(&op)) {
      DEBUG("SEND FAILED");
      return;
    }
    DEBUG("RECEIVED");
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
      DEBUG("SEND FAILED");
      return;
    }
    if (op.recv.r1 + op.recv.r2 == i * 2 && op.recv.r1 - op.recv.r2 == 10) {
      DEBUG("GOT RIGHT ANSWER");
    } else {
      DEBUG("GOT WRONG ANSWER!!");
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