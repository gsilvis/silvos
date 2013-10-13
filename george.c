#include "vga.h"
#include "threads.h"

void print_mem (void *mem) {
  puts("Welcome to GeorgeOS, Multiboot Edition!\r\n");
  yield();
  puts("Total Available Ram: ");
  puti((int) mem);
  puts(" MB\r\n");
  yield();
  yield();
  yield();
  yield();
  yield();
  yield();
  yield();
  yield();
  exit();
  __asm__("hlt"); /* not run */
  yield();
  yield();
}
char memstack[4096];

void welcome (void __attribute__ ((unused)) *a) {
  puts("YOOOOOOOOOOOOOOO!\r\n");
  yield();
  puts("How AAAAAAAARE you!\r\n");
  yield();
  while (1) {
    puts("Have a nice day!\r\n");
    yield();
  }
}
char welstack[4096];

void dumb1 (int n) {
  if (n == 0) {
    //    yield();
    putc('A');
    yield();
  } else {
    dumb1(n-1);
    putc('a');
    yield();
    dumb1(n-1);
  }
}
void dumb (void *i) {
  dumb1((int) i);
  exit();
}
char dumbstack[4096];

void moron (void __attribute__ ((unused)) *a) {
  for (int i = 0; i < 90000; i++) {
    //    puts("nooooo me fiiiiiirst!\r\n");
    putc('B');
    yield();
    yield();
  }
}
char moronstack[4096];
char moronstack2[4096];

void kernel_main (int magic, unsigned int *mboot_struct) {
  clear_screen();
  if (magic != 0x2BADB002) {
    puts("OOF!  Multiboot fail!\r\n");
    return;
  }
  //  puts("Welcome to GeorgeOS, Multiboot Edition!\r\nTotal Available Ram: ");
  int mem = (mboot_struct[1] + mboot_struct[2])/1024; /* Low mem + High mem */
//  puti((mboot_struct[1] + mboot_struct[2])/1024); /* Low mem + High mem */
//  puts(" MB\r\n");
  thread_create(&memstack[4088], print_mem, (void *)mem);
  thread_create(&welstack[4088], welcome, (void *)0);
  thread_create(&dumbstack[4088], dumb, (void *)20);
  thread_create(&moronstack[4088], moron, (void *)0);
  thread_create(&moronstack2[4088], moron, (void *)0);
  yield(); /* Initialize the thread system.  Does not return */
}
