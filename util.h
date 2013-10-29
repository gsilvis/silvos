#ifndef __SILVOS_UTIL_H
#define __SILVOS_UTIL_H

void __attribute__ ((noreturn)) panic (const char *s);
void blab (void);
void delay (unsigned int t);
int read_key (void);

#endif
