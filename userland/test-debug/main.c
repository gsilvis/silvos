#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void main (void) {
  DEBUG("Abracadabra");
  DEBUG("川端康成");
  if (60 != DEBUG("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!???")) {
    DEBUG("bad");
  }
}
