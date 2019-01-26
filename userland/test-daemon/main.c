#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

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
    DEBUG("echoing back message");
    msgs.send = msgs.recv;
  }
}

void main (void) {
  const unsigned long long NEW_STACK = 0x785000;
  if (palloc((const void *)NEW_STACK)) {
    DEBUG("palloc failed");
    return;
  }
  int child = spawn_daemon(echo, (void *)(NEW_STACK+0x1000));
  if (child < 0) {
    DEBUG("spawn_daemon failed");
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
    DEBUG("call failed");
    return;
  }
  if (msgs.send.addr != msgs.recv.addr) {
    DEBUG("message addr changed");
  }
  if (msgs.send.r1 != msgs.recv.r1) {
    DEBUG("message r1 changed");
  }
  if (msgs.send.r2 != msgs.recv.r2) {
    DEBUG("message r2 changed");
  }
}
