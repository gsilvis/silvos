#include "pci.h"

#include "util.h"
#include "vga.h"
#include "ide.h"

void initialize_device (uint8_t bus, uint8_t device, uint8_t function) {
  uint32_t class = pci_read(2, function, device, bus);
  if ((0xFFFF0000 & class) == 0x01010000) {
    ide_device_register(bus, device, function);
  }
}

void check_device (uint8_t bus, uint8_t device) {
  uint8_t function = 0;
  uint32_t vendor_device = pci_read(0, function, device, bus);
  if ((0xFFFF & vendor_device) == 0xFFFF) {
    return;
  }
  initialize_device(bus, device, function);
  uint32_t bluh = pci_read(3, function, device, bus);
  if ((0x00800000 & bluh) == 0x00800000) {
    /* Multi-function device */
    for (function = 1; function < 8; function++) {
      uint32_t vendor_device = pci_read(0, function, device, bus);
      if ((0xFFFF & vendor_device) == 0xFFFF) {
        continue;
      }
      initialize_device(bus, device, function);
    }
  }
}

void check_all_pci_busses (void) {
  for (unsigned int bus = 0; bus < 256; bus++) {
    for (unsigned int dev = 0; dev < 32; dev++) {
      check_device(bus, dev);
    }
  }
}


