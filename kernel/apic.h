#ifndef __SILVOS_APIC_H
#define __SILVOS_APIC_H

#include <stdint.h>

void apic_init (void);
void ap_apic_init (void);

void ioapic_init (void);

void apic_eoi (void);
void apic_send_start_ipi (uint32_t dest);

#endif
