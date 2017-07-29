#include "ide.h"

#include "alloc.h"
#include "pci.h"
#include "vga.h"
#include "ata.h"
#include "memory-map.h"
#include "pagefault.h"

#include <stdint.h>

uint8_t ide_bus;
uint8_t ide_device;
uint8_t ide_function;

uint16_t bar0, bar1, bar2, bar3, bar4;

prdt_entry *prdt;

uint8_t *ide_buf;

uint32_t ide_get_reg (uint8_t reg) {
  return pci_read(reg, ide_function, ide_device, ide_bus);
}

void ide_device_register (uint8_t bus, uint8_t device, uint8_t function) {
  puts("Found IDE device.\r\n");
  ide_bus = bus;
  ide_device = device;
  ide_function = function;
  bar0 = pci_read(4, function, device, bus);
  bar1 = pci_read(5, function, device, bus);
  bar2 = pci_read(6, function, device, bus);
  bar3 = pci_read(7, function, device, bus);
  bar4 = pci_read(8, function, device, bus) - 1;
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

  prdt = (prdt_entry *)allocate_phys_page();
  ide_buf = (uint8_t *)allocate_phys_page();

  uint32_t tmp = pci_read(1, function, device, bus);
  tmp |= 0x04;
  pci_write(1, function, device, bus, tmp);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, 0x00);
}

uint8_t checkide (void) {
  return inb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS);
}

int read_sector (uint64_t sector, void *to) {
  prdt[0].prd_phys_addr = virt_to_phys((uint64_t)ide_buf);
  prdt[0].xr_bytes = 512;
  prdt[0].is_last = IDE_PRDT_ENTRY_IS_LAST;

  uint64_t prdt_base = virt_to_phys((uint64_t)prdt);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT, 0xFF & prdt_base);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT+1, 0xFF & (prdt_base >> 8));
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT+2, 0xFF & (prdt_base >> 16));
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT+3, 0xFF & (prdt_base >> 24));
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, ATA_BUSMASTER_CMD_RW);

  outb(bar0 + ATA_REG_HDDEVSEL, 0x40);
  outb(bar0 + ATA_REG_SECCOUNT0, 0);
  outb(bar0 + ATA_REG_LBA0, 0xFF & (sector >> 24));
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 32));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 40));
  outb(bar0 + ATA_REG_SECCOUNT0, 1);
  outb(bar0 + ATA_REG_LBA0, 0xFF & sector);
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 8));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 16));
  outb(bar0 + ATA_REG_COMMAND, ATA_CMD_READ_DMA_EXT);

  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, ATA_BUSMASTER_CMD_RW | ATA_BUSMASTER_CMD_START);

  uint8_t status;
  while (1) {
    status = inb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS);
    if (ATA_BUSMASTER_SR_SENT_IRQ & status) {
      outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_SENT_IRQ);
      outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, 0x00);
      break;
    }
  }
  if (ATA_BUSMASTER_SR_FAIL & status) {
    outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_FAIL);
    goto err;
  }
  return copy_to_user(to, ide_buf, 512);
 err:
  return -1;
}


int write_sector (uint64_t sector, const void *from) {
  int err = 0;
  if ((err = copy_from_user(ide_buf, from, 512))) {
    return err;
  };
  prdt[0].prd_phys_addr = virt_to_phys((uint64_t)ide_buf);
  prdt[0].xr_bytes = 512;
  prdt[0].is_last = IDE_PRDT_ENTRY_IS_LAST;

  uint64_t prdt_base = virt_to_phys((uint64_t)prdt);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT, 0xFF & prdt_base);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT+1, 0xFF & (prdt_base >> 8));
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT+2, 0xFF & (prdt_base >> 16));
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_PRDT+3, 0xFF & (prdt_base >> 24));
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, 0x00);

  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_FAIL);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_SENT_IRQ);

  outb(bar0 + ATA_REG_HDDEVSEL, 0x40);
  outb(bar0 + ATA_REG_SECCOUNT0, 0);
  outb(bar0 + ATA_REG_LBA0, 0xFF & (sector >> 24));
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 32));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 40));
  outb(bar0 + ATA_REG_SECCOUNT0, 1);
  outb(bar0 + ATA_REG_LBA0, 0xFF & sector);
  outb(bar0 + ATA_REG_LBA1, 0xFF & (sector >> 8));
  outb(bar0 + ATA_REG_LBA2, 0xFF & (sector >> 16));
  outb(bar0 + ATA_REG_COMMAND, ATA_CMD_WRITE_DMA_EXT);

  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, ATA_BUSMASTER_CMD_START);

  uint8_t status;
  while (1) {
    status = inb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS);
    if (ATA_BUSMASTER_SR_SENT_IRQ & status) {
      outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_SENT_IRQ);
      outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, 0x00);
      break;
    }
  }
  if (ATA_BUSMASTER_SR_FAIL & status) {
    outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_FAIL);
    goto err;
  }
  return 0;
 err:
  return -1;
}

