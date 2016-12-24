#include "acpi.h"

#include "memory-map.h"
#include "util.h"
#include "vga.h"

#include <stdint.h>

struct RSDPDescriptor {
  char Signature[8];
  uint8_t Checksum;
  char OEMID[6];
  uint8_t Revision;
  uint32_t RsdtAddress;
} __attribute__ ((packed));

struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed));

struct RSDT {
  struct ACPISDTHeader h;
  uint32_t TablePointers[];
} __attribute__ ((packed));

struct FADT {
  struct ACPISDTHeader h;
  uint32_t FACS;
  uint32_t DSDT;
  /* Much more stuff, that I don't care about yet. */
} __attribute__ ((packed));

struct RSDPDescriptor *rsdp = NULL;
struct RSDT *rsdt = 0;
struct FADT *fadt = 0;
struct ACPISDTHeader *dsdt = 0;
struct ACPISDTHeader *ssdt = 0;
struct ACPISDTHeader *madt = 0;
struct ACPISDTHeader *hpet = 0;

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
  for (int i = 0; i < 4; i++) {
    putc(table->Signature[i]);
  }
  if (acpi_checksum((uint8_t *)table, table->Length)) {
    putc('\r');
    putc('\n');
    return -1; /* Bad checksum */
  }
  putc('!');
  putc('\r');
  putc('\n');
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
    hpet = table;
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
