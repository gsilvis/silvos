#include "hpet.h"

#include "acpi.h"
#include "memory-map.h"

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
  uint64_t config = hpet_reg->timers[0].config_and_cap;
  config &= ~HPET_TIMER_CONFIG_INTERRUPT_TYPE;  /* Edge-triggred */
  config &= ~HPET_TIMER_CONFIG_PERIODIC;        /* One-shot */
  config &= ~HPET_TIMER_CONFIG_FORCE_32;        /* 64-bit */
  config |= HPET_TIMER_CONFIG_INTERRUPT_ENABLE; /* Enabled */
  hpet_reg->timers[0].config_and_cap = config;
  return 0;
}

void hpet_reset_timeout (void) {
  uint64_t cur = hpet_reg->main_counter;
  hpet_reg->timers[0].comparator = cur + timeslice;
}
