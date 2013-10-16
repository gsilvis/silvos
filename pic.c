#include "bits.h"

#define MASTER_PIC 0x20
#define MASTER_PIC_COMMAND MASTER_PIC
#define MASTER_PIC_DATA (MASTER_PIC+1)
#define SLAVE_PIC 0xA0
#define SLAVE_PIC_COMMAND SLAVE_PIC
#define SLAVE_PIC_DATA (SLAVE_PIC+1)

void remap_pic (void) {
  outb(0x20, 0x11); /* Initialize and require 4th byte. */
  outb(0x21, 0x20); /* Offset ISRs 0 - 7 to interrupt lines 0x20 - 0x27. */
  outb(0x21, 0);    /* Do not cascade. */
  outb(0x21, 0x01); /* Enter 8086 mode. */
  outb(0x21, 0xFD); /* Block non-keyboard lines */
  return;
}
