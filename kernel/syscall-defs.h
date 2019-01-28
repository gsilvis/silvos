#ifndef __SILVOS_SYSCALL_DEFS_H
#define __SILVOS_SYSCALL_DEFS_H

typedef unsigned long long syscall_arg;

typedef enum {
  MESSAGE_RECEIVED = 0,
  SEND_FAILED = 1,
} sendrecv_status;

typedef unsigned long long semaphore_id;

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
#define SYSCALL_CALL        0x0C
#define SYSCALL_RESPOND     0x0D
#define SYSCALL_SEM_CREATE  0x0E
#define SYSCALL_SEM_DELETE  0x0F
#define SYSCALL_SEM_WATCH   0x10
#define SYSCALL_SEM_UNWATCH 0x11
#define SYSCALL_SEM_WAIT    0x12
#define SYSCALL_SEM_SET     0x13
#define SYSCALL_FIND_PROC   0x14
#define SYSCALL_SPAWN_DAEMON 0x15
#define SYSCALL_SET_HANDLER 0x16
#define SYSCALL_GET_TID     0x17
#define SYSCALL_FORK_DAEMON 0x18
#define SYSCALL_GET_IOPORT  0x19

#define NUM_SYSCALLS        0x20

#endif
