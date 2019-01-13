#ifndef __SILVOS_SYSCALL_DEFS_H
#define __SILVOS_SYSCALL_DEFS_H

typedef unsigned long long syscall_arg;

typedef enum {
  MESSAGE_RECEIVED = 0,
  SEND_FAILED = 1,
} sendrecv_status;

#define SYSCALL_YIELD       0x00
#define SYSCALL_PUTCH       0x01
#define SYSCALL_EXIT        0x02
#define SYSCALL_GETCH       0x03
#define SYSCALL_READ        0x04
#define SYSCALL_WRITE       0x05
#define SYSCALL_PALLOC      0x06
#define SYSCALL_PFREE       0x07
#define SYSCALL_DEBUG       0x08
#define SYSCALL_NANOSLEEP   0x09
#define SYSCALL_FORK        0x0A
#define SYSCALL_SPAWN       0x0B
#define SYSCALL_SENDRECV    0x0C

#define NUM_SYSCALLS        0x0D

#endif
