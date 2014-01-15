#ifndef __SILVOS_USERLAND_H
#define __SILVOS_USERLAND_H

inline void yield (void) {
  __asm__("int $0x36" :: "r"(0x36));
}

#endif
