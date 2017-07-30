#ifndef __SILVOS_VGA_H
#define __SILVOS_VGA_H

#include <stdint.h>

void putc (char c);
void puts (const char *c);
void clear_screen (void);

/* Make sure to put a \r\n at the end of your vga_printfs! */
int vga_printf (const char *fmt, ...);

#endif
