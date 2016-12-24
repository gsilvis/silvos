#include "vga.h"
#include "threads.h"
#include "idt.h"
#include "gdt.h"
#include "util.h"
#include "isr.h"
#include "pic.h"
#include "page.h"
#include "alloc.h"
#include "kbd.h"
#include "fpu.h"
#include "pci.h"
#include "memory-map.h"
#include "com.h"

void initialize_idt (void) {
  create_idt();

  /* Exceptions */
  register_isr(0x00, fault_isr, 0); /* #DE: Divide by Zero */
    /* 0x01  #DB: Debug */
    /* 0x02       Non-Maskable Interrupt */
    /* 0x03  #BP: Breakpoint Exception */
  register_isr(0x04, fault_isr, 0); /* #OF: Overflow */
  register_isr(0x05, fault_isr, 0); /* #BR: Bound Range Exceeded */
  register_isr(0x06, fault_isr, 0); /* #UD: Invalid Opcode */
  register_isr(0x07, nm_isr, 0);    /* #NM: Device Not Available */
  register_isr(0x08, df_isr, 0);    /* #DF: Double Fault */
    /* 0x09       Coprocessor Segment Overrun */
    /* 0x0A  #TS: Invalid TSS */
    /* 0x0B  #NP: Segment Not Present */
  register_isr(0x0C, fault_isr, 0); /* #SS: Stack Fault */
  register_isr(0x0D, fault_isr, 0); /* #GP: General Protection */
  register_isr(0x0E, pf_isr, 1);    /* #PF: Page Fault */
    /* 0x0F       [undefined] */
  register_isr(0x10, fault_isr, 0); /* #MF: x87 FPE */
    /* 0x11  #AC: Alignment Check */
    /* 0x12  #MC: Machine Check */
  register_isr(0x13, fault_isr, 0); /* #XM: SIMD FPE */
    /* 0x14  #VE: Virtualization Exception */

  /* IRQ */
  register_isr(0x20, timer_isr, 0);
  register_isr(0x21, kbd_isr, 0);

  /* Syscalls */
  register_isr(0x36, yield_isr, 0);
  register_isr(0x37, putch_isr, 0);
  register_isr(0x38, exit_isr, 0);
  register_isr(0x39, getch_isr, 0);
  register_isr(0x3A, read_isr, 0);
  register_isr(0x3B, write_isr, 0);
  register_isr(0x3C, palloc_isr, 0);
  register_isr(0x3D, pfree_isr, 0);
  register_isr(0x3E, debug_isr, 0);

}

typedef struct {
  uint32_t start;
  uint32_t end;
  uint32_t string;
  uint32_t unused;
} multiboot_module;

void kernel_main (uint32_t mboot_struct_addr) {
  uint32_t *mboot_struct =
      (uint32_t *)phys_to_virt((uint64_t)mboot_struct_addr);
  memtop = ((uint64_t)mboot_struct[2]) * 1024;
  memtop += 0x100000;
  clear_screen();
  puts("Welcome to GeorgeOS, Multiboot Edition!\r\n");
  initialize_allocator();
  insert_pt(initial_pt());
  initialize_idt();
  insert_idt();
  initialize_gdt();
  remap_pic();

  multiboot_module *mod_list =
      (multiboot_module *)phys_to_virt((uint64_t)mboot_struct[6]);
  for (uint32_t i = 0; i < mboot_struct[5]; i++) {
    user_thread_create((void *)phys_to_virt((uint64_t)mod_list[i].start),
                       mod_list[i].end - mod_list[i].start);
  }

  idle_thread_create();
  init_kbd();
  fpu_init();
  check_all_pci_busses();
  com_initialize();
  puts("Launching userspace.\r\n");
  schedule(); /* Does not return */
}
