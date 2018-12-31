#ifndef __SILVOS_ACPI_H
#define __SILVOS_ACPI_H

#include <stdint.h>
#include <stddef.h>

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

struct ACPIAddress {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
} __attribute__ ((packed));

static const uint8_t ACPI_ADDRESS_SPACE_SYSTEM_MEMORY = 0;
static const uint8_t ACPI_ADDRESS_SPACE_SYSTEM_IO     = 1;
static const uint8_t ACPI_ADDRESS_SPACE_PCI_CONFIG    = 2;
static const uint8_t ACPI_ADDRESS_SPACE_EMBEDDED      = 3;
static const uint8_t ACPI_ADDRESS_SPACE_SMBUS         = 4;

static const uint8_t ACPI_ADDRESS_ACCESS_SIZE_UNDEFINED = 0;
static const uint8_t ACPI_ADDRESS_ACCESS_SIZE_BYTE      = 1;
static const uint8_t ACPI_ADDRESS_ACCESS_SIZE_SHORT     = 2;
static const uint8_t ACPI_ADDRESS_ACCESS_SIZE_WORD      = 3;
static const uint8_t ACPI_ADDRESS_ACCESS_SIZE_QUAD      = 4;

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

struct HPET {
  struct ACPISDTHeader h;
  uint32_t timer_id;
  struct ACPIAddress base;
  uint8_t hpet_number;
  uint16_t min_periodic_tick;
  uint8_t page_protection;
} __attribute__ ((packed));

struct MADT {
  struct ACPISDTHeader h;
  uint32_t local_interrupt_controller_address;
  uint32_t flags;
  /* More stuff */
} __attribute__ ((packed));

static const uint8_t HPET_PAGE_PROTECTION_NONE = 0;
static const uint8_t HPET_PAGE_PROTECTION_4K   = 1;
static const uint8_t HPET_PAGE_PROTECTION_64K  = 2;

struct RSDPDescriptor *rsdp;
struct RSDT *rsdt;
struct FADT *fadt;
struct HPET *hpet;
struct ACPISDTHeader *dsdt;
struct ACPISDTHeader *ssdt;
struct MADT *madt;


int acpi_initialize (void);

#endif
