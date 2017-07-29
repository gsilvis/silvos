#include "acpi.h"

#include "memory-map.h"
#include "util.h"
#include "vga.h"

#include <stdint.h>

uint8_t acpi_checksum (uint8_t *table, uint64_t length) {
  uint8_t check = 0;
  for (uint64_t i = 0; i < length; i++) {
    check += table[i];
  }
  return check;
}

int acpi_find_rsdp (void) {
  /* TODO: search extended BIOS area as well */
  for (uint64_t ptr = 0xE0000; ptr < 0x100000; ptr += 16) {
    if (strncmp((char *)phys_to_virt(ptr), "RSD PTR ", 8)) {
      continue;
    }
    if (acpi_checksum((uint8_t *)phys_to_virt(ptr), sizeof(struct RSDPDescriptor))) {
      continue;
    }
    rsdp = (struct RSDPDescriptor *)phys_to_virt(ptr);
    return 0;
  }
  return -1; /* Not found */
}

int acpi_parse_rsdp (void) {
  if (rsdp->Revision) {
    return -1; /* I don't know how to handle ACPI version > 1.0 */
  }
  rsdt = (struct RSDT *)phys_to_virt((uint64_t)rsdp->RsdtAddress);
  return 0;
}

int acpi_parse_table (struct ACPISDTHeader *table) {
  puts("ACPI ");
  for (int i = 0; i < 4; i++) {
    putc(table->Signature[i]);
  }
  putc(' ');
  if (acpi_checksum((uint8_t *)table, table->Length)) {
    puts("Invalid Checksum!\r\n");
    return -1; /* Bad checksum */
  }
  puts("Init\r\n");
  if (!strncmp(table->Signature, "RSDT", 4)) {
    rsdt = (struct RSDT *)table;
    uint32_t num_entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;
    for (uint32_t i = 0; i < num_entries; i++) {
      acpi_parse_table((struct ACPISDTHeader *)phys_to_virt((uint64_t)rsdt->TablePointers[i]));
    }
  } else if (!strncmp(table->Signature, "FACP", 4)) {
    fadt = (struct FADT *)table;
    /* Don't parse the FACS, for now */
    acpi_parse_table((struct ACPISDTHeader *)phys_to_virt((uint64_t)fadt->DSDT));
  } else if (!strncmp(table->Signature, "DSDT", 4)) {
    dsdt = table;
  } else if (!strncmp(table->Signature, "SSDT", 4)) {
    ssdt = table;
  } else if (!strncmp(table->Signature, "APIC", 4)) {
    madt = table;
  } else if (!strncmp(table->Signature, "HPET", 4)) {
    hpet = (struct HPET *)table;
  }
  return 0;
}

int acpi_initialize (void) {
  int error = 0;
  if ((error = acpi_find_rsdp()))  return error;
  if ((error = acpi_parse_rsdp()))  return error;
  if ((error = acpi_parse_table(&rsdt->h)))  return -1;
  /* Check that all the actually required tables (none for now) are there */
  return 0;
}
