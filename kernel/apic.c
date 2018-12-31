#include "apic.h"

#include <stdint.h>
#include <stddef.h>

#include "acpi.h"
#include "vga.h"

enum MadtEntryType {
  MADT_ENTRY_LOCAL_APIC = 0,
  MADT_ENTRY_IO_APIC = 1,
  MADT_ENTRY_ISA_OVERRIDE = 2,
  MADT_ENTRY_NMI_OVERRIDE = 4,
};

/* Returns the size of the entry */
static uint8_t analyze_madt_entry (uint8_t *entry) {
  uint16_t *entry_16 = (uint16_t *)entry;
  uint32_t *entry_32 = (uint32_t *)entry;
  uint8_t entry_type = entry[0];
  uint8_t entry_size = entry[1];
  /* TODO: Print bytes with the hh specifier once it's supported. */
  switch (entry_type) {
    case MADT_ENTRY_LOCAL_APIC:
      vga_printf("CPU-Local APIC for CPU %hd with ID %hd\r\n", entry[2], entry[3]);
      if (entry_32[1] == 0) {
        vga_printf("THIS CPU IS DISABLED!?\r\n");
      }
      break;
    case MADT_ENTRY_IO_APIC:
      vga_printf("IO APIC %hd at 0x%X with interrupt base %d\r\n", entry[2], entry_32[1], entry_32[2]);
      break;
    case MADT_ENTRY_ISA_OVERRIDE: {
      uint8_t isa_line = entry[3];
      uint32_t remapped_apic_line = entry_32[1];
      uint16_t flags = entry_16[4];
      uint8_t polarity_is_low = ((flags & 0x03) == 0x01);
      uint8_t trigger_is_level = ((flags & 0x0C) == 0x11);
      vga_printf("ISA IRQ Override from %hd to %d with flags 0x%hX (low=%d level=%d)\r\n", isa_line, remapped_apic_line, flags, polarity_is_low, trigger_is_level);
      break;
    }
    case MADT_ENTRY_NMI_OVERRIDE: {
      uint16_t flags = ((uint16_t)entry[3]) * 256 + (uint16_t)entry[4];
      if (entry[2] == 255) {
        vga_printf("Local APIC NMI structure for all CPUs: flags 0x%hX, NMI is line %hd\r\n", flags, entry[5]);
      } else {
        vga_printf("Local APIC NMI structure for CPU %hd: flags 0x%hX, NMI is line %hd\r\n", entry[2], flags, entry[5]);
      }
      break;
    }
    default:
      vga_printf("Unimplemented MADT entry with type: %hd\r\n", entry_type);
      break;
  }
  return entry_size;
}

void apic_init (void) {
  vga_printf("CPU-Local APIC is at 0x%X\r\n", madt->local_interrupt_controller_address);

  const uint32_t total_length = madt->h.Length;
  uint8_t *slice = (uint8_t *)madt;
  uint32_t offset = sizeof(struct MADT);
  while (offset < total_length) {
    offset += analyze_madt_entry(&slice[offset]);
  }
}
