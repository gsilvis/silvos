#include "userland.h"

#define DEBUG(str) debug(str, sizeof(str))

void main (void) {
  char *my_page = (char *)0xC0000000;
  palloc(my_page);
  char *j = &my_page[0];
  char *k = &my_page[512];
  j[0] = 'H';
  j[1] = 'e';
  j[2] = 'l';
  j[3] = 'l';
  j[4] = 'o';
  j[5] = '!';
  j[6] = '!';
  k[0] = 'G';
  k[1] = 'o';
  k[2] = 'o';
  k[3] = 'd';
  k[4] = 'b';
  k[5] = 'y';
  k[6] = 'e';
  if (write(0, &j[0]))  return;
  if (write(1, &k[0]))  return;
  if (read(0, &k[0]))  return;
  if (read(1, &j[0]))  return;
  if (k[0] != 'H')  return;
  if (k[1] != 'e')  return;
  if (k[2] != 'l')  return;
  if (k[3] != 'l')  return;
  if (k[4] != 'o')  return;
  if (k[5] != '!')  return;
  if (k[6] != '!')  return;
  if (j[0] != 'G')  return;
  if (j[1] != 'o')  return;
  if (j[2] != 'o')  return;
  if (j[3] != 'd')  return;
  if (j[4] != 'b')  return;
  if (j[5] != 'y')  return;
  if (j[6] != 'e')  return;
  DEBUG("OK");

  char *not_my_page = (char *)0xE0000000;
  if (-1 != write(0, &not_my_page[0]))  DEBUG("bad1");
  if (-1 != read(0, &not_my_page[0]))  DEBUG("bad2");
}
