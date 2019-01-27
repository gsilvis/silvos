#include "userland-lib.h"

void main() {
  /* Basic formatting */
  debug_printf("Hello, %s!", "World");
  debug_printf("Ed ball%c", 's');
  debug_printf("The magic number is %d", 42);
  debug_printf("%u%% chance of failure", 100u);
  debug_printf("Some longs: %lu %ld", 1099511627776lu, -1099511627776l);
  debug_printf("I speak hex: 0x%x 0x%lx", 0x42, 0x1234567890abcdefl);
  debug_printf("I CAN SHOUT: 0x%X 0x%lX", 0xaaaa, 0xa1aea1aea1ael);
  debug_printf("0%o is the best permission", 0777);
  debug_printf("Number of times I've printfed a short: %hi", (short)1);

  /* Padding (subtle.) */
  debug_printf("|%15s|", "short");
  debug_printf("|%15s|", "loooooooooooooooooooooooooooooong");
  debug_printf("|%15d|", 1);
  debug_printf("|%15d|", 0);
  debug_printf("|%15d|", 10);
  debug_printf("|%+15d|", 10);
  debug_printf("|%15d|", -1);
  debug_printf("|%015d|", 10);
  debug_printf("|%+015d|", 10);
  debug_printf("|%015d|", -42);
  debug_printf("|% 015d|", 10);
  debug_printf("|%15p|", 0xdeadbeef);

  /* And left-justified... */
  debug_printf("|%-15s|", "short");
  debug_printf("|%-15d|", 1);
  debug_printf("|%-15d|", 0);
  debug_printf("|%-15d|", 10);
  debug_printf("|%-+15d|", 10);
  debug_printf("|%-15d|", -1);

  /* The zero-filling doesn't have an effect here. */
  debug_printf("|%-015d|", 10);
  debug_printf("|%-+015d|", 10);
  debug_printf("|%-015d|", -42);
  debug_printf("|%- 015d|", 10);
  debug_printf("|%-15p|", 0xdeadbeef);
}
