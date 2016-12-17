#include "ide.h"

#include "pci.h"
#include "vga.h"
#include "ata.h"
#include "pagefault.h"

#include <stdint.h>

uint8_t ide_bus;
uint8_t ide_device;
uint8_t ide_function;

uint16_t bar0, bar1, bar2, bar3, bar4;


void ide_device_register (uint8_t bus, uint8_t device, uint8_t function) {
  puts("Found IDE device.\r\n");
  bar0 = pci_read(4, function, device, bus);
  bar1 = pci_read(5, function, device, bus);
  bar2 = pci_read(6, function, device, bus);
  bar3 = pci_read(7, function, device, bus);
  bar4 = pci_read(8, function, device, bus);
  if ((bar0 & 0xFFFFFFFE) == 0x00) {
    bar0 = IDE_DEFAULT_BAR0;
  }
  if ((bar1 & 0xFFFFFFFE) == 0x00) {
    bar1 = IDE_DEFAULT_BAR1;
  }
  if ((bar2 & 0xFFFFFFFE) == 0x00) {
    bar2 = IDE_DEFAULT_BAR2;
  }
  if ((bar3 & 0xFFFFFFFE) == 0x00) {
    bar3 = IDE_DEFAULT_BAR3;
  }
  put_int(bar0);
  puts("\r\n");
  put_int(bar1);
  puts("\r\n");
  put_int(bar2);
  puts("\r\n");
  put_int(bar3);
  puts("\r\n");
  put_int(bar4);
  puts("\r\n");
  puts("\r\n\r\n");
  outb(bar0 + ATA_REG_HDDEVSEL, 0xA0);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  put_byte(inb(bar0 + 0x07));
  puts(" ");
  put_short(inw(bar0));
  puts("\r\n");

  outb(bar0 + ATA_REG_HDDEVSEL, 0xB0);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  put_byte(inb(bar0 + 0x07));
  puts(" ");
  put_short(inw(bar0));
  puts("\r\n");

  outb(bar2 + ATA_REG_HDDEVSEL, 0xA0);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  put_byte(inb(bar2 + 0x07));
  puts(" ");
  put_short(inw(bar0));
  puts("\r\n");

  outb(bar2 + ATA_REG_HDDEVSEL, 0xB0);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  put_byte(inb(bar2 + 0x07));
  puts(" ");
  put_short(inw(bar0));
  puts("\r\n");
}

int read_sector (uint64_t sector, void *to) {
  outb(bar0 + ATA_REG_HDDEVSEL, 0x40);
  outb(bar0 + ATA_REG_SECCOUNT0, 0);
  outb(bar0 + ATA_REG_LBA0, 0xFF & (sector >> 24));
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 32));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 40));
  outb(bar0 + ATA_REG_SECCOUNT0, 1);
  outb(bar0 + ATA_REG_LBA0, 0xFF & sector);
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 8));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 16));
  outb(bar0 + ATA_REG_COMMAND, ATA_CMD_READ_PIO_EXT);
  uint8_t status;
  do {
    status = inb(bar0 + ATA_REG_STATUS);
    if (ATA_SR_BSY & status) {
      continue;
    } else if (!(ATA_SR_DRQ & status)) {
      continue;
    } else if (ATA_SR_ERR & status) {
      goto err;
    } else {
      break;
    }
  } while (1);
  uint16_t tmp[256];
  for (int i = 0; i < 256; i++) {
      tmp[i] = inw(bar0);
  }
  return copy_to_user(to, tmp, 512);
 err:
  return -1;
}


int write_sector (uint64_t sector, const void *from) {
  uint16_t tmp[256];
  int err = 0;
  if ((err = copy_from_user(tmp, from, 512))) {
    return err;
  };
  outb(bar0 + ATA_REG_HDDEVSEL, 0x40);
  outb(bar0 + ATA_REG_SECCOUNT0, 0);
  outb(bar0 + ATA_REG_LBA0, 0xFF & (sector >> 24));
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 32));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 40));
  outb(bar0 + ATA_REG_SECCOUNT0, 1);
  outb(bar0 + ATA_REG_LBA0, 0xFF & sector);
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 8));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 16));
  outb(bar0 + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO_EXT);
  uint8_t status;
  do {
    status = inb(bar0 + ATA_REG_STATUS);
    if (ATA_SR_BSY & status) {
      continue;
    } else if (!(ATA_SR_DRQ & status)) {
      continue;
    } else if (ATA_SR_ERR & status) {
      goto err;
    } else {
      break;
    }
  } while (1);
  for (int i = 0; i < 256; i++) {
    outw(bar0, tmp[i]);
  }
  return 0;
 err:
  return -1;
}

