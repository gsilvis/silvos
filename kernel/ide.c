#include "ide.h"

#include "alloc.h"
#include "ata.h"
#include "memory-map.h"
#include "pagefault.h"
#include "pci.h"
#include "threads.h"
#include "vga.h"

#include <stdint.h>

static uint16_t bar0, bar1, bar2, bar3, bar4;

static prdt_entry *prdt;

static uint8_t *ide_buf;

static tcb *operating_tcb;  /* NULL if no operation in progress. */
static uint8_t is_read;

void ide_device_register (uint8_t bus, uint8_t device, uint8_t function) {
  /* For now, assume that these are all ports.  If the low bit is not set, then
   * they are actually memory addresses. */
  bar0 = pci_read(4, function, device, bus) & 0xFFFC;
  bar1 = pci_read(5, function, device, bus) & 0xFFFC;
  bar2 = pci_read(6, function, device, bus) & 0xFFFC;
  bar3 = pci_read(7, function, device, bus) & 0xFFFC;
  bar4 = pci_read(8, function, device, bus) & 0xFFFC;
  if (bar0 == 0x00) {
    bar0 = IDE_DEFAULT_BAR0;
  }
  if (bar1 == 0x00) {
    bar1 = IDE_DEFAULT_BAR0;
  }
  if (bar2 == 0x00) {
    bar2 = IDE_DEFAULT_BAR0;
  }
  if (bar3 == 0x00) {
    bar3 = IDE_DEFAULT_BAR0;
  }
  outb(bar0 + ATA_REG_HDDEVSEL, 0xA0);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);

  outb(bar0 + ATA_REG_HDDEVSEL, 0xB0);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);
  inb(bar0 + 0x07);

  outb(bar2 + ATA_REG_HDDEVSEL, 0xA0);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);

  outb(bar2 + ATA_REG_HDDEVSEL, 0xB0);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);
  inb(bar2 + 0x07);

  prdt = (prdt_entry *)allocate_phys_page();
  ide_buf = (uint8_t *)allocate_phys_page();

  uint32_t tmp = pci_read(1, function, device, bus);
  tmp |= 0x04;
  pci_write(1, function, device, bus, tmp);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, 0x00);
  operating_tcb = 0;

  /* TODO: Find size of disk, and don't let users try and read/write off the
   * end of it. */
}

uint8_t checkide (void) {
  return inb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS);
}

void __attribute__((noreturn)) read_sector (void) {
  if (operating_tcb != 0) {
    panic("Concurrent IDE read...");
  }
  memset(ide_buf, '\0', 512);  /* In case we read off the end of the disk,
                                  don't leak information to the end-user. */
  uint64_t sector = running_tcb->saved_registers.rbx;
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

  operating_tcb = running_tcb;
  is_read = 1;
  schedule();
}

void __attribute__((noreturn)) write_sector (void) {
  if (operating_tcb != 0) {
    panic("Concurrent IDE write...");
  }
  uint64_t sector = running_tcb->saved_registers.rbx;
  uint8_t *user_addr = (uint8_t *)running_tcb->saved_registers.rcx;
  if (copy_from_user(ide_buf, user_addr, 512)) {
    running_tcb->saved_registers.rax = -1;
    return_to_current_thread();
  }
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

  operating_tcb = running_tcb;
  is_read = 0;
  schedule();
}

void ide_handler (void) {
  uint8_t status = inb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS);
  if (!(ATA_BUSMASTER_SR_SENT_IRQ & status)) {
    return;  /* Spurious IRQ. */
  }
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_STATUS, ATA_BUSMASTER_SR_SENT_IRQ);
  outb(bar4 + ATA_BUSMASTER_REG_PRIMARY_COMMAND, 0x00);
  if (operating_tcb == 0) {
    panic("Unexpected IDE IRQ!");
  }
  reschedule_thread(operating_tcb);
  if (ATA_BUSMASTER_SR_FAIL & status) {
    /* Report failure to user. */
    operating_tcb->saved_registers.rax = -1;
    operating_tcb = 0;
  } else if (!is_read) {
    /* Successfully completed write. */
    operating_tcb->saved_registers.rax = 0;
    operating_tcb = 0;
  } else {
    /* Successfully completed read; will be finished in ide_finish_operation. */
  }
}

void ide_finish_operation (void) {
  if ((running_tcb == operating_tcb) && is_read) {
    operating_tcb = 0;
    uint8_t *user_addr = (uint8_t *)running_tcb->saved_registers.rcx;
    if (copy_to_user(user_addr, ide_buf, 512)) {
      running_tcb->saved_registers.rax = -1;
    } else {
      running_tcb->saved_registers.rax = 0;
    }
  }
}
