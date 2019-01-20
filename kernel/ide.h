#ifndef __SILVOS_IDE_H
#define __SILVOS_IDE_H

#include <stdint.h>

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define IDE_DEFAULT_BAR0 0x01F0
#define IDE_DEFAULT_BAR1 0x03F4
#define IDE_DEFAULT_BAR2 0x0170
#define IDE_DEFAULT_BAR3 0x0374

typedef struct {
  uint32_t prd_phys_addr;
  uint16_t xr_bytes; /* 0 for 2**16 */
  uint16_t is_last;
}  __attribute__((packed)) prdt_entry;

#define IDE_PRDT_ENTRY_IS_LAST   0x8000
#define IDE_PRDT_ENTRY_NOT_LAST  0x0000

void ide_device_register(uint8_t bus, uint8_t device, uint8_t function);

void __attribute__((noreturn)) read_sector (void);
void __attribute__((noreturn)) write_sector (void);

void ide_finish_operation (void);

#endif
