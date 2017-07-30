#include "pci.h"

#include "ide.h"
#include "vga.h"
#include "util.h"

typedef void (*pci_device_init)(uint8_t bus, uint8_t device, uint8_t function);

typedef struct pci_handler_s {
  uint16_t class_subclass;
  pci_device_init init;
  const char *name;
} pci_handler;

static pci_handler pci_handlers[] = {
  { PCI_CLASS_IDE_CONTROLLER, ide_device_register, "IDE Controller" },
  { PCI_CLASS_HOST_BRIDGE, NULL, "Host Bridge" },
  { PCI_CLASS_ISA_BRIDGE, NULL, "ISA Bridge" },
  { PCI_CLASS_OTHER_BRIDGE, NULL, "Other Bridge" },
  { PCI_CLASS_VGA_CONTROLLER, NULL, "VGA Controller" },
  { PCI_CLASS_ETHERNET_CONTROLLER, NULL, "Ethernet Controller" },
  { 0, NULL, NULL },
};

void initialize_device (uint8_t bus, uint8_t device, uint8_t function, uint16_t vendor) {
  uint16_t class_subclass = pci_read(2, function, device, bus) >> 16;
  const char *name = "Unknown";
  pci_device_init initializer = NULL;
  for (pci_handler *h = pci_handlers; h->name != NULL; h++) {
    if (class_subclass == h->class_subclass) {
      initializer  = h->init;
      name = h->name;
      break;
    }
  }
  vga_printf("PCI %02x:%02x.%02x (class: %04hx vendor: %04hx) %s%s\r\n",
             bus, device, function, class_subclass, vendor,
             name, initializer == NULL ? " (Unhandled)" : "");
  if (initializer != NULL) {
    initializer(bus, device, function);
  }
}

void check_device (uint8_t bus, uint8_t device) {
  uint8_t function = 0;
  uint16_t vendor = pci_read(0, function, device, bus) & 0xFFFF;
  if (vendor == PCI_VENDOR_INVALID) {
    return;
  }
  initialize_device(bus, device, function, vendor);
  uint8_t header_type = (pci_read(3, function, device, bus) >> 16) & 0xFF;
  if (0x80 & header_type) {
    /* Multi-function device */
    for (function = 1; function < PCI_NUM_FUNCTION; function++) {
      uint16_t vendor = pci_read(0, function, device, bus) & 0xFFFF;
      if (vendor == PCI_VENDOR_INVALID) {
        continue;
      }
      initialize_device(bus, device, function, vendor);
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
