#include "bits.h"

void set_timeout (void) {
  outb(0x43, 0x30); /* Set Channel 0 to two-byte mode, interrupt on terminal count */
  outb(0x40, 0xFF); /* Set timeout to 65535 pulses */
  outb(0x40, 0xFF); /* About 50 ms */
}
