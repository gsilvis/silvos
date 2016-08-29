#ifndef __SILVOS_UTIL_H
#define __SILVOS_UTIL_H

void memset (char *ptr, char byte, unsigned int count);
void memcpy (unsigned char *src, unsigned char *dest, unsigned int count);
void __attribute__ ((noreturn)) panic (const char *s);
void blab (void);
void delay (unsigned int t);
int read_key (void);

#endif
