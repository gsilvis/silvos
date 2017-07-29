#ifndef __SILVOS_PCI_H
#define __SILVOS_PCI_H

#include "util.h"

#include <stdint.h>

#define PCI_ADDRESS 0x0CF8
#define PCI_DATA    0x0CFC

#define PCI_NUM_BUS 256
#define PCI_NUM_DEV 32
#define PCI_NUM_FUNCTION 8

#define PCI_VENDOR_INVALID 0xFFFF

#define PCI_CLASS_IDE_CONTROLLER 0x0101
#define PCI_CLASS_HOST_BRIDGE 0x0600
#define PCI_CLASS_ISA_BRIDGE 0x0601
#define PCI_CLASS_OTHER_BRIDGE 0x0680
#define PCI_CLASS_VGA_CONTROLLER 0x0300
#define PCI_CLASS_ETHERNET_CONTROLLER 0x0200

static inline void pci_set_addr (uint8_t register_num,
                                 uint8_t function_num,
                                 uint8_t device_num,
                                 uint8_t bus_num) {
  uint32_t address = 0x80000000;
  address |= (uint32_t)register_num << 2;
  address |= (uint32_t)function_num << 8;
  address |= (uint32_t)device_num << 11;
  address |= (uint32_t)bus_num << 16;

  outd(PCI_ADDRESS, address);
}

static inline uint32_t pci_read_current_addr (void) {
  return ind(PCI_DATA);
}

static inline void pci_write_current_addr (uint32_t data) {
  outd(PCI_DATA, data);
}

static inline uint32_t pci_read (uint8_t register_num,
                                 uint8_t function_num,
                                 uint8_t device_num,
                                 uint8_t bus_num) {
  pci_set_addr(register_num, function_num, device_num, bus_num);
  return pci_read_current_addr();
}

static inline void pci_write (uint8_t register_num,
                              uint8_t function_num,
                              uint8_t device_num,
                              uint8_t bus_num,
                              uint32_t data) {
  pci_set_addr(register_num, function_num, device_num, bus_num);
  pci_write_current_addr(data);
}

void check_all_pci_busses (void);

#endif
