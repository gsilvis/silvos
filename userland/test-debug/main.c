#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void main (void) {
  DEBUG("Abracadabra");
  DEBUG("川端康成");
  if (60 != DEBUG("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!???")) {
    DEBUG("bad");
  }
  if (debug((char *)0xFFFFFFFF80000000, 128))  DEBUG("bad-kernel");
  if (debug((char *)0xC0000000, 10))  DEBUG("bad-pagefault");
}
