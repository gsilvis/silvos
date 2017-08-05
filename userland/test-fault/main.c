#include "userland.h"

#include <stdint.h>

#define DEBUG(str) debug(str, sizeof(str))

/* the number of possible faults */
static int max_faults = 5;

void main (void) {
  int id;

  for (id = 0; id < max_faults; id++) {
    if (fork() == 0) {
      nanosleep(100000);
      break;
    }
  }

  char h[] = "FAULTING X...";
  h[9] = '0' + id;

  DEBUG(h);

  switch(id) {
  case 0:
    __asm__ volatile ("int $0x38"); /* something undefined */
    break;
  case 1:
    __asm__ volatile ("int $0x21"); /* keyboard isr */
    break;
  case 2:
    __asm__ volatile ("mov %rax, %cr4"); /* general protection */
    break;
  case 3:
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0), "Nd" (0x20)); /* PIC port */
    break;
  case 4:
    __asm__ volatile ("hlt");
    break;
  default:
    DEBUG("unused branch");
    return;
  }

  DEBUG("FAILED TO FAULT!!!");
}
