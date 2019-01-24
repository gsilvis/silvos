#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void handler_task (void) {
  sendrecv_op op;
  op.send.addr = 0;
  op.send.r1 = 0;
  op.send.r2 = 0;
  while (1) {
    if (respond(&op)) {
      DEBUG("RESPOND FAILED");
      return;
    }
    op.send.addr = op.recv.addr;
    /* Other args are ignored... */
    if (palloc((void *)(op.recv.r1 & 0xFFFFFFFFFFFFF000))) {
      DEBUG("PALLOC FAILED");
      return;
    }
    char my_char = (char)((0xFF000 & op.recv.r1) >> 12);
    char msg[] = "Write character: _";
    msg[sizeof(msg) - 2] = my_char;
    DEBUG(msg);
    char *addr = (char *)op.recv.r1;
    *addr = my_char;
  }
}

void main (void) {
  const long long HANDLER_STACK = 0x10000000;
  if (palloc((const void *)HANDLER_STACK)) {
    DEBUG("ALLOC FAIL");
    return;
  }
  int handler_id = spawn_daemon(handler_task, (void *)(HANDLER_STACK + 0x1000));
  if (handler_id < 0) {
    DEBUG("SPAWN FAIL");
  }
  set_handler(handler_id);
  for (int k = 0; k < 6; k++) {
    char *addr = (char *)(0x41000L + k * 0x4000L);
    char msg[] = "Read character: _";
    msg[sizeof(msg) - 2] = *addr;
    DEBUG(msg);
  }
  for (int k = 0; k < 6; k++) {
    char *addr = (char *)(0x41000L + k * 0x4000L);
    char msg[] = "Read character: _";
    msg[sizeof(msg) - 2] = *addr;
    DEBUG(msg);
  }
}
