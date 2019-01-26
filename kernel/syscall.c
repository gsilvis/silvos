#include "com.h"
#include "hpet.h"
#include "ide.h"
#include "ipc.h"
#include "kbd.h"
#include "multiboot.h"
#include "palloc.h"
#include "semaphores.h"
#include "syscall-defs.h"
#include "threads.h"
#include "vga.h"

typedef void __attribute__((noreturn)) (*syscall_func) (void);

/* Syscall functions must never return.  Therefore, here are some helpers for
 * wrapping methods that always return a value (without switching threads or
 * anything), and making them instead write that value into the saved
 * registers, and return to the current thread.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wint-conversion"

#define TRAMPOLINE_NAME(n) syscall_trampoline_ ## n

#define TRAMPOLINE_HELPER(n, expr) \
  static void __attribute__((noreturn)) TRAMPOLINE_NAME(n) (void) { \
    uint64_t a = running_tcb->saved_registers.rbx; \
    uint64_t b = running_tcb->saved_registers.rcx; \
    running_tcb->saved_registers.rax = expr; \
    return_to_current_thread(); \
  }

#define TRAMPOLINE0(n) TRAMPOLINE_HELPER(n, n())
#define TRAMPOLINE0_NORET(n) TRAMPOLINE_HELPER(n, (n(), 0))
#define TRAMPOLINE1(n) TRAMPOLINE_HELPER(n, n(a))
#define TRAMPOLINE1_NORET(n) TRAMPOLINE_HELPER(n, (n(a), 0))
#define TRAMPOLINE2(n) TRAMPOLINE_HELPER(n, n(a, b))
#define TRAMPOLINE2_NORET(n) TRAMPOLINE_HELPER(n, (n(a, b), 0))

/* Trampoline definitions */

TRAMPOLINE1_NORET(putc)
TRAMPOLINE1(palloc)
TRAMPOLINE1(pfree)
TRAMPOLINE2(com_debug_thread)
TRAMPOLINE0(fork)
TRAMPOLINE2(spawn_within_vm_space)
TRAMPOLINE0(sem_create)
TRAMPOLINE1(sem_delete)
TRAMPOLINE1(sem_watch)
TRAMPOLINE1(sem_unwatch)
TRAMPOLINE1(sem_set)
TRAMPOLINE1(find_module)
TRAMPOLINE1(set_handler)
TRAMPOLINE0(get_tid)

#pragma GCC diagnostic pop

/* Dispatch table definitions */

syscall_func syscall_defns[NUM_SYSCALLS] = {
  yield,
  TRAMPOLINE_NAME(putc),
  thread_exit,
  getch,
  read_sector,
  write_sector,
  TRAMPOLINE_NAME(palloc),
  TRAMPOLINE_NAME(pfree),
  TRAMPOLINE_NAME(com_debug_thread),
  hpet_nanosleep,
  TRAMPOLINE_NAME(fork),
  TRAMPOLINE_NAME(spawn_within_vm_space),
  call,
  respond,
  TRAMPOLINE_NAME(sem_create),
  TRAMPOLINE_NAME(sem_delete),
  TRAMPOLINE_NAME(sem_watch),
  TRAMPOLINE_NAME(sem_unwatch),
  sem_wait,
  TRAMPOLINE_NAME(sem_set),
  TRAMPOLINE_NAME(find_module),
  spawn_daemon_within_vm_space,
  TRAMPOLINE_NAME(set_handler),
  TRAMPOLINE_NAME(get_tid),
  fork_daemon,
};

void __attribute__((noreturn)) syscall_handler (void) {
  uint64_t syscall_num = running_tcb->saved_registers.rax;
  if (syscall_num >= NUM_SYSCALLS) {
    running_tcb->saved_registers.rax = -1;
    return_to_current_thread();
  } else {
    syscall_defns[syscall_num]();
  }
}
