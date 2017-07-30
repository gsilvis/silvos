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

/* List of known class/subclasses, and how to handle them.  At some point this
 * information may not be specific enough to determine how to use a given
 * device, but it's okay for now. */
static pci_handler pci_handlers[] = {
  { PCI_CLASS_IDE_CONTROLLER, ide_device_register, "IDE Controller" },
  { PCI_CLASS_ETHERNET_CONTROLLER, NULL, "Ethernet Controller" },
  { PCI_CLASS_VGA_CONTROLLER, NULL, "VGA Controller" },
  { PCI_CLASS_AUDIO_CONTROLLER, NULL, "Audio Controller" },
  { PCI_CLASS_HOST_BRIDGE, NULL, "Host Bridge" },
  { PCI_CLASS_ISA_BRIDGE, NULL, "ISA Bridge" },
  { PCI_CLASS_OTHER_BRIDGE, NULL, "Other Bridge" },
  { 0, NULL, NULL },
};

/* Returns non-zero if the function exists. */
uint8_t initialize_function (uint8_t bus, uint8_t device, uint8_t function) {
  uint32_t vendor_product = pci_read(0, function, device, bus);
  uint16_t vendor = vendor_product;
  uint16_t product = vendor_product >> 16;
  if (vendor == PCI_VENDOR_INVALID) {
    return 0;
  }
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
  vga_printf("PCI %02x:%02x.%01x  [%04hx] %-20s  [%04hx:%04hx] %s\r\n",
             bus, device, function, class_subclass, name, vendor,
             product, initializer == NULL ? " (Unhandled)" : "");
  if (initializer != NULL) {
    initializer(bus, device, function);
  }
  return 1;
}

void initialize_device (uint8_t bus, uint8_t device) {
  if (!initialize_function(bus, device, 0)) {
    return;
  }
  /* If this is a single-function device, then only look at function 0.  Some
   * single-function devices report the same device on every function, rather
   * than only reporting it on 0. */
  uint8_t header_type = pci_read(3, 0, device, bus) >> 16;
  if (!(header_type & 0x80)) {
    return;
  }
  for (uint8_t fn = 1; fn < PCI_NUM_FUNCTION; fn++) {
    initialize_function(bus, device, fn);
  }
}

void check_all_pci_busses (void) {
  for (unsigned int bus = 0; bus < PCI_NUM_BUS; bus++) {
    for (unsigned int dev = 0; dev < PCI_NUM_DEV; dev++) {
      initialize_device(bus, dev);
    }
  }
}
