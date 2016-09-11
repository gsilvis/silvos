#ifndef __SILVOS_VGA_H
#define __SILVOS_VGA_H

#include <stdint.h>

void putc (char c);
void clear_screen (void);
void puts (const char *s);
void puti (uint32_t i);

void put_byte (uint8_t d);
void put_short (uint16_t d);
void put_int (uint32_t d);

#endif
