#include "printf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void __attribute__ ((noreturn)) panic (const char *s) {
  printf("Panic! %s\n", s);
  exit(-1);
}

static char my_output[1000];
static int my_ix;
void putch2(char c) {
  putchar(c);
  my_output[my_ix++] = c;
}

void test_vprintf(const char *fmt, ...) {
  int my_ret;
  my_ix = 0;
  {
    va_list argp;
    va_start(argp, fmt);
    my_ret = kvprintf(putch2, fmt, argp);
    va_end(argp);
  }
  int their_ret;
  char their_output[1000];
  {
    va_list argp;
    va_start(argp, fmt);
    their_ret = vsprintf(their_output, fmt, argp);
    va_end(argp);
  }
  if (my_ret != their_ret) {
    printf("Error on %s: %d != %d\n", fmt, my_ret, their_ret);
  }
  my_output[my_ix] = 0;
  int res = strcmp(my_output, their_output);
  if (res != 0) {
    printf("Error on %s: %s != %s\n", fmt, my_output, their_output);
  }
}

int main() {
  /* Basic formatting */
  test_vprintf("Hello, %s!\n", "World");
  test_vprintf("Ed ball%c\n", 's');
  test_vprintf("The magic number is %d\n", 42);
  test_vprintf("%u%% chance of failure\n", 100u);
  test_vprintf("Some longs: %lu %ld\n", 1099511627776lu, -1099511627776l);
  test_vprintf("I speak hex: 0x%x 0x%lx\n", 0x42, 0x1234567890abcdefl);
  test_vprintf("I CAN SHOUT: 0x%X 0x%lX\n", 0xaaaa, 0xa1aea1aea1ael);
  test_vprintf("0%o is the best permission\n", 0777);
  test_vprintf("Number of times I've printfed a short: %hi\n", (short)1);

  /* Padding (subtle.) */
  test_vprintf("|%15s|\n", "short");
  test_vprintf("|%15s|\n", "loooooooooooooooooooooooooooooong");
  test_vprintf("|%15d|\n", 1);
  test_vprintf("|%15d|\n", 0);
  test_vprintf("|%15d|\n", 10);
  test_vprintf("|%+15d|\n", 10);
  test_vprintf("|%15d|\n", -1);
  test_vprintf("|%015d|\n", 10);
  test_vprintf("|%+015d|\n", 10);
  test_vprintf("|%015d|\n", -42);
  test_vprintf("|% 015d|\n", 10);
  test_vprintf("|%15p|\n", 0xdeadbeef);

  /* And left-justified... */
  test_vprintf("|%-15s|\n", "short");
  test_vprintf("|%-15d|\n", 1);
  test_vprintf("|%-15d|\n", 0);
  test_vprintf("|%-15d|\n", 10);
  test_vprintf("|%-+15d|\n", 10);
  test_vprintf("|%-15d|\n", -1);

  /* The zero-filling doesn't have an effect here. */
  test_vprintf("|%-015d|\n", 10);
  test_vprintf("|%-+015d|\n", 10);
  test_vprintf("|%-015d|\n", -42);
  test_vprintf("|%- 015d|\n", 10);
  test_vprintf("|%-15p|\n", 0xdeadbeef);

  /* Old bug: Check format string ending in format specifier. */
  test_vprintf("5 4 3 2 %d", 1);  /* no newline */
  test_vprintf(" 0\n");
}
