#include "userland.h"

void main (void) {
  debug("Abracadabra");
  debug("川端康成");
  if (60 != debug("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!???")) {
    debug("bad");
  }
  if (_debug((char *)0xFFFFFFFF80000000, 10))  debug("bad-kernel");
  if (_debug((char *)0xC0000000, 10))  debug("bad-pagefault");
}
