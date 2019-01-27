#include "userland.h"

void echo (void) {
  sendrecv_op msgs = {
    .send = {
      .addr = 0,
      .r1 = 0,
      .r2 = 0,
    },
  };
  while (1) {
    respond(&msgs);
    debug("echoing back message");
    msgs.send = msgs.recv;
  }
}

void main (void) {
  const unsigned long long NEW_STACK = 0x785000;
  if (palloc((const void *)NEW_STACK)) {
    debug("palloc failed");
    return;
  }
  int child = spawn_daemon(echo, (void *)(NEW_STACK+0x1000));
  if (child < 0) {
    debug("spawn_daemon failed");
    return;
  }
  /* Don't yield here! */
  sendrecv_op msgs = {
    .send = {
      .addr = child,
      .r1 = 5,
      .r2 = 6,
    },
  };
  if (call(&msgs)) {
    debug("call failed");
    return;
  }
  if (msgs.send.addr != msgs.recv.addr) {
    debug("message addr changed");
  }
  if (msgs.send.r1 != msgs.recv.r1) {
    debug("message r1 changed");
  }
  if (msgs.send.r2 != msgs.recv.r2) {
    debug("message r2 changed");
  }
}
