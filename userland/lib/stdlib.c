#include <stddef.h>
#include <stdint.h>

void memset (void *ptr, char byte, size_t count) {
  char *p = ptr;
  for (unsigned int i = 0; i < count; i++) {
    p[i] = byte;
  }
}

void memcpy (void *dest, const void *src, size_t count) {
  char *d = dest;
  const char *s = src;
  for (unsigned int i = 0; i < count; i++) {
    d[i] = s[i];
  }
}

int strncmp (const char *a, const char *b, size_t n) {
  for (unsigned int i = 0; i < n; i++) {
    if (a[i] < b[i]) {
      return -1;
    } else if (a[i] > b[i]) {
      return 1;
    }
  }
  return 0;
}

uint64_t strlen (const char *s) {
  uint64_t i = 0;
  while (s[i]) i++;
  return i;
}

char *strncpy (char *dest, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }
  for (; i < n; ++i) {
    dest[i] = '\0';
  }
  /* No trailing null byte on truncation, as per <string.h> */
  return dest;
}
