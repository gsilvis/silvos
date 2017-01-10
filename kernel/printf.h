#ifndef __SILVOS_PRINTF_H
#define __SILVOS_PRINTF_H

#include <stdarg.h>

/* We support the following standard printf format specifiers:
 * %[optional flags][optional pad width][xXodiupsc%][optional length h/l]
 * Default lengths are assumed to be 32 bits wide. Note in particular the lack
 * of floating-point support and therefore the length specifier L. We also do
 * not support precision modifiers. */
int vprintf (void (*my_putch)(char), const char *fmt, va_list argp);

#endif
