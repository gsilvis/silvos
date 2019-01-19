#ifndef __SILVOS_APIC_H
#define __SILVOS_APIC_H

#include <stdint.h>

void apic_init (void);
void ap_apic_init (void);

/* Register a handler for an ISA device, and unmask that line so we can receive
 * interrupts. */
void register_isa_isr (uint8_t isa_line, void (*handler)(void));

void apic_eoi (void);

#endif
