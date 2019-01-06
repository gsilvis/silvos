#include "syscall.h"

#include "com.h"
#include "hpet.h"
#include "ide.h"
#include "ipc.h"
#include "kbd.h"
#include "palloc.h"
#include "threads.h"
#include "vga.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wint-conversion"

#define TRAMPOLINE_NAME(n) syscall_trampoline_ ## n

#define TRAMPOLINE_HELPER(n, x) \
  static syscall_arg TRAMPOLINE_NAME(n) ( syscall_arg a, syscall_arg b ) { \
    syscall_arg res = 0; \
    x;\
    return res; \
  }

#define TRAMPOLINE0(n) TRAMPOLINE_HELPER(n, res = n())
#define TRAMPOLINE0_NORET(n) TRAMPOLINE_HELPER(n, n())
#define TRAMPOLINE1(n) TRAMPOLINE_HELPER(n, res = n(a))
#define TRAMPOLINE1_NORET(n) TRAMPOLINE_HELPER(n, n(a))
#define TRAMPOLINE2(n) TRAMPOLINE_HELPER(n, res = n(a, b))
#define TRAMPOLINE2_NORET(n) TRAMPOLINE_HELPER(n, n(a, b))

/* Trampoline definitions */

TRAMPOLINE0_NORET(yield)
TRAMPOLINE1_NORET(putc)
TRAMPOLINE0_NORET(thread_exit)
TRAMPOLINE0(getch)
TRAMPOLINE2(read_sector)
TRAMPOLINE2(write_sector)
TRAMPOLINE1(palloc)
TRAMPOLINE1(pfree)
TRAMPOLINE2(com_debug_thread)
TRAMPOLINE1_NORET(hpet_nanosleep)
TRAMPOLINE2_NORET(spawn_within_vm_space)
TRAMPOLINE1(sendrecv)

#pragma GCC diagnostic pop

/* Dispatch table definitions */

syscall_func syscall_defns[] = {
  TRAMPOLINE_NAME(yield),
  TRAMPOLINE_NAME(putc),
  TRAMPOLINE_NAME(thread_exit),
  TRAMPOLINE_NAME(getch),
  TRAMPOLINE_NAME(read_sector),
  TRAMPOLINE_NAME(write_sector),
  TRAMPOLINE_NAME(palloc),
  TRAMPOLINE_NAME(pfree),
  TRAMPOLINE_NAME(com_debug_thread),
  TRAMPOLINE_NAME(hpet_nanosleep),
  /* fork *can't* have any C in it's stack. */
  (syscall_func)fork,
  TRAMPOLINE_NAME(spawn_within_vm_space),
  TRAMPOLINE_NAME(sendrecv),
};

uint64_t syscall_defns_len = sizeof(syscall_defns)/sizeof(syscall_func);

