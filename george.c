#include "vga.h"
#include "threads.h"
#include "idt.h"
#include "gdt.h"
#include "util.h"
#include "bits.h"
#include "isr.h"
#include "pic.h"
#include "page.h"
#include "alloc.h"

void forever_yielding (void *a) {
  char c = (char)(unsigned int) a;
  while (1) {
    putc(c);
    yield();
  }
}

void forever_unyielding (void *a) {
  char c = (char)(unsigned int) a;
  while (1) {
    putc(c);
    delay(4000000);
  }
}

void initialize_idt (void) {
  create_idt();
  for (int i = 0; i < 256; i++) {
    register_isr(i, isr_other);
  }
  register_isr(0x35, dumb_isr);
  register_isr(0x36, yield_isr);
  register_isr(0x07, spurious_isr);
  register_isr(0x08, doublefault_isr);
  register_isr(0x20, timer_isr);
  register_isr(0x21, kbd_isr);
}

void create_test_threads (void) {
  int *(stacks[4]);
  for (int i = 0; i < 4; i++) {
    stacks[i] = allocate_phys_page();
  }
  thread_create(&stacks[0][1024], forever_yielding, (void *)'u');
  thread_create(&stacks[1][1024], forever_yielding, (void *)'v');
  thread_create(&stacks[2][1024], forever_unyielding, (void *)'A');
  thread_create(&stacks[3][1024], forever_unyielding, (void *)'B');
}

void kernel_main (int magic, unsigned int *mboot_struct) {
  clear_screen();
  if (magic != 0x2BADB002) {
    puts("OOF!  Multiboot fail!\r\n");
    return;
  }


  puts("Welcome to GeorgeOS, Multiboot Edition!\r\n");
  puts("Total Available Ram: ");
  puti((mboot_struct[1] + mboot_struct[2])/1024); /* Low mem + High mem */
  puts(" MB\r\n");

  puts("Initializing memory allocator\r\n");
  initialize_allocator(mboot_struct[2]);
  puts("Initializing IDT\r\n");
  initialize_idt();
  puts("Inserting IDT\r\n");
  insert_idt();
  puts("Inserting GDT\r\n");
  insert_gdt();
  puts("Remapping PIC\r\n");
  remap_pic();
  puts("Enabling paging\r\n");
  enable_paging();
  puts("Creating test threads\r\n");
  create_test_threads();
  puts("Enabling hardware interrupts\r\n");
  sti();
  puts("Initializing thread subsystem\r\n");
  yield(); /* Does not return */
}
