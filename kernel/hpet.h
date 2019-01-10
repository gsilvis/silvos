#ifndef __SILVOS_HPET_H
#define __SILVOS_HPET_H

#include <stdint.h>

struct HPET_Timer {
  uint64_t config_and_cap;
  uint64_t comparator;
  uint64_t fsb_interrupt;
  uint64_t unused;
} __attribute__ ((packed));

static const uint64_t HPET_TIMER_CONFIG_INTERRUPT_TYPE   = 0x0002;  /* 1 == level-triggered */
static const uint64_t HPET_TIMER_CONFIG_INTERRUPT_ENABLE = 0x0004;
static const uint64_t HPET_TIMER_CONFIG_PERIODIC         = 0x0008;
static const uint64_t HPET_TIMER_CAPABILITY_PERIODIC     = 0x0010;
static const uint64_t HPET_TIMER_CAPABILITY_SIZE         = 0x0020;  /* 1 == 64-bit */
static const uint64_t HPET_TIMER_CONFIG_PERIOD_SET       = 0x0040;
static const uint64_t HPET_TIMER_CONFIG_FORCE_32         = 0x0100;
static const uint64_t HPET_TIMER_CONFIG_ROUTE_MASK       = 0x3E00;
static const uint64_t HPET_TIMER_CONFIG_FSB_ENABLE       = 0x4000;
static const uint64_t HPET_TIMER_CAPABILITY_FSB          = 0x8000;


struct HPET_Registers {
  uint64_t capabilities;
  uint64_t skip0;
  uint64_t general_config;
  uint64_t skip1;
  uint64_t interrupt_status;
  uint64_t skip2;
  uint64_t skip[24];
  uint64_t main_counter;
  uint64_t skip3;
  struct HPET_Timer timers[];
} __attribute__ ((packed));

static const uint64_t HPET_CAPABILITIES_LAST_TIMER_MASK = 0x1F00;
static const uint64_t HPET_CAPABILITIES_SIZE            = 0x2000;  /* 1 == 64-bit */
static const uint64_t HPET_CAPABILITIES_LEGACY_ROUTE    = 0x8000;
static const uint64_t HPET_CAPABILITIES_CLK_PERIOD      = 0xFFFFFFFF00000000;  /* in femtoseconds */

static const uint64_t HPET_GENERAL_CONFIG_ENABLE        = 0x01;
static const uint64_t HPET_GENERAL_CONFIG_LEGACY_ROUTE  = 0x02;

volatile struct HPET_Registers *hpet_reg;

int hpet_initialize (void);
void hpet_reset_timeout (void);

void hpet_nanosleep (void);
void hpet_sleepers_awake(void);

#endif
