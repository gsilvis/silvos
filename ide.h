#ifndef __SILVOS_IDE_H
#define __SILVOS_IDE_H

#include <stdint.h>

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define IDE_DEFAULT_BAR0 0x01F0
#define IDE_DEFAULT_BAR1 0x03F6
#define IDE_DEFAULT_BAR2 0x0170
#define IDE_DEFAULT_BAR3 0x0376

void ide_device_register(uint8_t bus, uint8_t device, uint8_t function);

#endif
