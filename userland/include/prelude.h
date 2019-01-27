#ifndef __SILVOS_PRELUDE_H
#define __SILVOS_PRELUDE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

void memset (void *ptr, char byte, size_t count);
void memcpy (void *dest, const void *src, size_t count);
int strncmp (const char *s1, const char *s2, size_t n);
int strcmp (const char *s1, const char *s2);
char *strncpy (char *dest, const char *src, size_t n);
char *strcpy (char *dest, const char *src);
size_t strlen (const char *s);

int vsprintf (char *buf, const char *fmt, va_list argp);

#endif
