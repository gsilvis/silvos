#ifndef __SILVOS_COM_H
#define __SILVOS_COM_H

#include <stddef.h>
#include <stdint.h>

void com_initialize (void);
void com_putch (char c);
void com_puts (const char *text);
void com_put_tid (void);
uint32_t com_debug_thread (char *text, uint32_t len);

#endif
