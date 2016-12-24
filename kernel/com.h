#ifndef __SILVOS_COM_H
#define __SILVOS_COM_H

#include <stddef.h>

void com_initialize (void);
void com_putch (char c);
void com_debug (char *text);

#endif
