#include "semaphores.h"

#include "com.h"
#include "list.h"
#include "syscall-defs.h"
#include "threads.h"
#include "util.h"

#define NUM_SEMAPHORES 64

typedef enum {
  SEM_IDLE,
  SEM_WATCHED,  /* watched by its owner and not set */
  SEM_SET,      /* if set and watched by owner, ready_sem is nonempty */
} sem_state;

typedef struct {
  /* either empty or part of a tcb's ready_sems list */
  struct list_head ready_sem;
  uint64_t id;
  uint64_t owner;  /* If zero, unused */
  sem_state state;
} semaphore;

semaphore sems[NUM_SEMAPHORES];

static semaphore *find_semaphore (semaphore_id id) {
  for (int i = 0; i < NUM_SEMAPHORES; ++i) {
    if (sems[i].owner != 0 && sems[i].id == id) {
      return &sems[i];
    }
  }
  return 0;
}

semaphore_id sem_create (void) {
  static uint64_t sem_id = 1;
  for (int i = 0; i < NUM_SEMAPHORES; ++i) {
    /* We don't actively delete semaphores on thread destruction, so do a
     * get_tcb call to check for orphans. */
    if (sems[i].owner == 0 || get_tcb(sems[i].owner) == 0) {
      sems[i].id = sem_id++;
      sems[i].owner = running_tcb->thread_id;
      sems[i].state = SEM_IDLE;
      sems[i].ready_sem.next = &sems[i].ready_sem;
      sems[i].ready_sem.prev = &sems[i].ready_sem;
      return sems[i].id;
    }
  }
  panic("Ran out of semaphores to give out.");
}

int sem_delete (semaphore_id id) {
  semaphore* sem = find_semaphore(id);
  if (!sem) return -1;
  if (sem->owner != running_tcb->thread_id) return -1;
  sem->owner = 0;
  list_remove(&sem->ready_sem);
  return 0;
}

int sem_watch (semaphore_id id) {
  semaphore* sem = find_semaphore(id);
  if (!sem) return -1;
  if (sem->owner != running_tcb->thread_id) return -1;
  switch (sem->state) {
    case SEM_WATCHED:
      return 0;
    case SEM_IDLE:
      sem->state = SEM_WATCHED;
      break;
    case SEM_SET:
      /* Figure out if we were already watching this sem. */
      if (!list_empty(&sem->ready_sem)) {
        return 0;
      }
      list_push_back(&sem->ready_sem, &running_tcb->ready_sems);
      break;
  }
  return 0;
}

int sem_unwatch (semaphore_id id) {
  semaphore* sem = find_semaphore(id);
  if (!sem) return -1;
  if (sem->owner != running_tcb->thread_id) return -1;
  switch (sem->state) {
    case SEM_IDLE:
      return 0;
    case SEM_WATCHED:
      sem->state = SEM_IDLE;
      break;
    case SEM_SET:
      list_remove(&sem->ready_sem);
      break;
  }
  return 0;
}

static void consume_semaphore(tcb* tcb) {
  if (list_empty(&tcb->ready_sems)) {
    panic("consume_semaphore woken up with no ready sems!");
  }
  if (tcb->sem_state != SEM_WAITING) {
    panic("tcb is consuming semaphore but not waiting!");
  }
  semaphore *sem = (semaphore*)list_pop_front(&tcb->ready_sems);
  if (sem->owner != tcb->thread_id) {
    panic("consume_semaphore called on unowned sem!");
  }
  if (sem->state != SEM_SET) {
    panic("consume_semaphore called on unset sem!");
  }
  sem->state = SEM_WATCHED;
  tcb->sem_state = SEM_NOT_WAITING;
  tcb->saved_registers.rax = sem->id;
}

void __attribute__((noreturn)) sem_wait (void) {
  running_tcb->sem_state = SEM_WAITING;
  if (list_empty(&running_tcb->ready_sems)) {
    schedule();
  } else {
    consume_semaphore(running_tcb);
    return_to_current_thread();
  }
}

int sem_set(semaphore_id id) {
  semaphore* sem = find_semaphore(id);
  if (sem->owner == 0) return -1;
  tcb* owner = get_tcb(sem->owner);
  if (!owner) return -1;

  switch (sem->state) {
    case SEM_IDLE:
      sem->state = SEM_SET;
      break;
    case SEM_WATCHED:
      sem->state = SEM_SET;
      list_push_back(&sem->ready_sem, &owner->ready_sems);
      if (owner->sem_state == SEM_WAITING) {
        consume_semaphore(owner);
        reschedule_thread(owner);
      }
      break;
    case SEM_SET:
      break;
  }
  return 0;
}
