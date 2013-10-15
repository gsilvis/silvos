#include "vga.h"
#include "threads.h"
#include "idt.h"
#include "util.h"
#include "bits.h"
#include "isr.h"

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
  yield();
  yield();
  //  panic("wefwef");

}

void welcome (void __attribute__ ((unused)) *a) {
  puts("YOOOOOOOOOOOOOOO!\r\n");
  yield();
  puts("How AAAAAAAARE you!\r\n");
  yield();
  while (1) {
    puts("Have a nice day!\r\n");
    __asm__("int $0x36");
  }
}

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
}

void moron (void __attribute__ ((unused)) *a) {
  for (int i = 0; i < 90000; i++) {
    //    puts("nooooo me fiiiiiirst!\r\n");
    putc('B');
    yield();
    yield();
  }
}

int stacks[5][1024];

void initialize_idt (void) {
  register_isr(0x35, dumb_isr);
  register_isr(0x36, yield_isr);
}

void kernel_main (int magic, unsigned int *mboot_struct) {
  clear_screen();
  if (magic != 0x2BADB002) {
    puts("OOF!  Multiboot fail!\r\n");
    return;
  }

  int mem = (mboot_struct[1] + mboot_struct[2])/1024; /* Low mem + High mem */
  thread_create(&stacks[0][1020], print_mem, (void *)mem);
  //  thread_create(&stacks[1][1020], welcome, (void *)0xdeadbeef);
  //  thread_create(&stacks[2][1020], dumb, (void *)20);
  thread_create(&stacks[3][1020], moron, (void *)0);
  //  thread_create(&stacks[4][1020], moron, (void *)0);

  puts("Initializing IDT");
  initialize_idt();
  puts("Inserting IDT");
  insert_idt();
  sti();

  yield(); /* Initialize the thread system.  Does not return */
}
