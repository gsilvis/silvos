#include "hpet.h"

#include "acpi.h"
#include "list.h"
#include "memory-map.h"
#include "threads.h"

uint64_t timeslice;

int hpet_initialize (void) {
  if (!hpet) {
    return -1;
  }
  if (ACPI_ADDRESS_SPACE_SYSTEM_MEMORY != hpet->base.AddressSpace) {
    return -2;  /* Not handling that right now */
  }
  hpet_reg = (struct HPET_Registers *)phys_to_virt(hpet->base.Address);
  hpet_reg->general_config |=
      HPET_GENERAL_CONFIG_ENABLE | HPET_GENERAL_CONFIG_LEGACY_ROUTE;
  timeslice = 50000000000000 / (hpet_reg->capabilities >> 32);
  /* PIT and RTC are now inactive */
  /* Set timers 0 and 1 to edge-triggered */
  for (int i = 0; i < 2; i++) {
    uint64_t config = hpet_reg->timers[i].config_and_cap;
    config &= ~HPET_TIMER_CONFIG_INTERRUPT_TYPE;  /* Edge-triggred */
    config &= ~HPET_TIMER_CONFIG_PERIODIC;        /* One-shot */
    config &= ~HPET_TIMER_CONFIG_FORCE_32;        /* 64-bit */
    config |= HPET_TIMER_CONFIG_INTERRUPT_ENABLE; /* Enabled */
    hpet_reg->timers[i].config_and_cap = config;
  }
  return 0;
}


/* Timeslice preemption, on timer 0 */

void hpet_reset_timeout (void) {
  uint64_t cur = hpet_reg->main_counter;
  hpet_reg->timers[0].comparator = cur + timeslice;
}


/* Sleep, on timer 1 */

struct sleeper {
  struct list_head queue;
  uint64_t deadline;
  tcb *thread;
};

LIST_HEAD(sleep_queue);

void hpet_nanosleep (uint64_t usecs) {
  uint64_t ticks = usecs * 1000000000 / (hpet_reg->capabilities >> 32);
  uint64_t cur = hpet_reg->main_counter;
  uint64_t deadline = cur + ticks;
  if (deadline < cur) {
    return;
  }

  struct list_head *i = sleep_queue.next;
  while (i != &sleep_queue) {
    struct sleeper *s = (struct sleeper *)i;
    if (deadline < s->deadline) {
      break;
    }
    i = i->next;
  }

  struct sleeper thread_sleeper = {
    .deadline = deadline,
    .thread = running_tcb,
  };
  list_push_back(&thread_sleeper.queue, i);

  /* I would love to just write to the register and call 'schedule()', but
   * there's an irritating race-condition: the main counter could pass our
   * deadline between when we read the main counter and when we wrote the
   * deadline.  So, we have to carefully write it, and check it afterwards... it
   * turns out that this is exactly the same logic we have to use on getting an
   * interrupt.  So just do that procedure. */
  hpet_sleepers_awake();

  /* Even if the thread isn't blocked, still deschedule it.  Users shouldn't be
   * sleeping for small enough amounts of time that they get control back
   * immediately. */
  schedule();
}

void hpet_sleepers_awake() {
  while (1) {
    struct list_head *i = sleep_queue.next;
    while (i != &sleep_queue) {
      uint64_t cur = hpet_reg->main_counter;
      struct sleeper *s = (struct sleeper *)i;
      if (s->deadline >= cur) {
        break;
      }
      i = i->next;
      list_remove(&s->queue);
      reschedule_thread(s->thread);
    }
    if (list_empty(&sleep_queue)) {
      return;
    }
    struct sleeper *s = (struct sleeper *)i;
    uint64_t current_deadline = s->deadline;
    hpet_reg->timers[1].comparator = current_deadline;
    /* According to a very frustrated-sounding comment in the Linux kernel,
     * some HPET implementations lag a bit before actually setting the timer.
     * Even if the deadline is in the future, if it's too soon in the future,
     * assume that the HPET still might not ever give as an interrupt for it.
     * Sad. */
    if (hpet_reg->main_counter + 8 < current_deadline) {
      return;
    }
    /* Dammit, the deadline passed in-between our last two reads of the main
     * register.  We don't know if we'll get an interrupt for it, so assume we
     * won't:  wake the sleepers again. */
  }
}
