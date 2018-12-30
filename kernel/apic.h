#ifndef __SILVOS_APIC_H
#define __SILVOS_APIC_H

void apic_init (void);
void ap_apic_init (void);

void ioapic_init (void);

void apic_eoi (void);

#endif
