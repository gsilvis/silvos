#include "pci.h"

#include "util.h"
#include "vga.h"
#include "ide.h"

void initialize_device (uint8_t bus, uint8_t device, uint8_t function) {
  uint16_t class_subclass = pci_read(2, function, device, bus) >> 16;
  if (class_subclass == PCI_CLASS_IDE_CONTROLLER) {
    ide_device_register(bus, device, function);
  }
}

void check_device (uint8_t bus, uint8_t device) {
  uint8_t function = 0;
  uint16_t vendor = pci_read(0, function, device, bus) & 0xFFFF;
  if (vendor == PCI_VENDOR_INVALID) {
    return;
  }
  initialize_device(bus, device, function);
  uint8_t header_type = (pci_read(3, function, device, bus) >> 16) & 0xFF;
  if (0x80 & header_type) {
    /* Multi-function device */
    for (function = 1; function < PCI_NUM_FUNCTION; function++) {
      uint16_t vendor = pci_read(0, function, device, bus) & 0xFFFF;
      if (vendor == PCI_VENDOR_INVALID) {
        continue;
      }
      initialize_device(bus, device, function);
    }
  }
}

void check_all_pci_busses (void) {
  for (unsigned int bus = 0; bus < PCI_NUM_BUS; bus++) {
    for (unsigned int dev = 0; dev < PCI_NUM_DEV; dev++) {
      check_device(bus, dev);
    }
  }
}
