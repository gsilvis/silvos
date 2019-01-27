#include "prelude.h"
#include "userland.h"

void main (void) {
  char *my_page = (char *)0xC0000000;
  palloc(my_page);
  char *j = &my_page[0];
  char *k = &my_page[512];
  strncpy(j, "Hello!!", sizeof("Hello!!"));
  strncpy(k, "Goodbye", sizeof("Goodbye"));
  debug(j);
  debug(k);
  if (write(0, &j[0]))  return;
  if (write(1, &k[0]))  return;
  if (read(0, &k[0]))  return;
  if (read(1, &j[0]))  return;
  debug(j);
  debug(k);
  if (strncmp(k, "Hello!!", strlen("Hello!!"))) return;
  if (strncmp(j, "Goodbye", strlen("Goodbye"))) return;
  debug("OK");

  char *not_my_page = (char *)0xE0000000;
  if (-1 != write(0, &not_my_page[0]))  debug("bad1");
  if (-1 != read(0, &not_my_page[0]))  debug("bad2");
}
