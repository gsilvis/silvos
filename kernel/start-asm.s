KERNEL_OFFSET = 0xFFFFFFFF80000000

MULTIBOOT_MAGIC = 0x1BADB002
MULTIBOOT_FLAGS = 0x00000003  /* Page align, memory info */
MULTIBOOT_CHECKSUM = -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

/* ****** MULTIBOOT HEADER ****** */

.section .multiboot, "a"
.GLOBAL mboot
mboot:
        .long MULTIBOOT_MAGIC
        .long MULTIBOOT_FLAGS
        .long MULTIBOOT_CHECKSUM
        .long mboot

/* ****** INITIALIZATION CODE ****** */

.section .inittext, "ax"
.code32
.GLOBAL _start
_start:
        /* Save multiboot information.  These variables are propagated all the
         * way to kernel_main. */
        mov %eax, %esi
        mov %ebx, %edi

        /* Load provisional GDT. */
        lgdt early_gdt_ptr

        /* Initialize segment registers from provisional GDT. */
        ljmp $0x10, $1f  /* 32-bit ring 0 code */
1:      mov $0x08, %ax                   /* ring 0 data */
        mov %ax, %ds
        mov %ax, %ss
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs

        /* Enable PAE. */
        mov %cr4, %eax
        orl $0x20, %eax
        mov %eax, %cr4

        /* Load our provisional page table. */
        mov $early_pml4, %eax
        mov %eax, %cr3

        /* Enable IA32e mode, and NX bit. */
        mov $0xC0000080, %ecx
        rdmsr
        orl $0x00000900, %eax
        wrmsr

        /* Enable paging. */
        mov %cr0, %eax
        orl $0x80000000, %eax
        movl %eax, %cr0

        /* Enter long mode! */
        ljmp $0x18, $_start64

.code64
.GLOBAL _start64
_start64:
        /* Jump to high memory! */
        mov $_start64_high, %rax
        jmp *%rax

.section .text
.GLOBAL _start64_high
_start64_high:
        /* Set up our stack, finally. */
        movq $stack,%rsp

        /* Initialize and load the real GDT. */
        pushq %rsi
        pushq %rdi
        call insert_gdt
        popq %rdi
        popq %rsi

        /* Load the real segment selectors. */
        pushq $0x40     /* 64-bit ring 0 code */
        pushq $1f
        lretq
1:      mov $0x10, %ax   /* ring 0 data */
        mov %ax, %ds
        mov %ax, %ss
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs

        /* Off we go!  kernel_main does not return. */
        call kernel_main

/* ****** DATA FOR USE IN EARLY BOOT ****** */

.section .initdata, "a"
early_gdt:
        .long 0x00000000, 0x00000000 /* Mandatory null descriptor */
        .long 0x00000000, 0x00CF9200 /* Data */
        .long 0x00000000, 0x00CF9A00 /* 32-bit ring 0 code */
        .long 0x00000000, 0x00A09B00 /* 64-bit ring 0 code */
early_gdt_ptr:
        .word 0x20 /* 4 entries */
        .long early_gdt

.section .initpt, "a"
.align 4096
early_pml4:
        .quad early_pdpt + 0x03
        .rept 510
                .quad 0
        .endr
        .quad early_pdpt + 0x03
early_pdpt:
        .quad early_pd + 0x03
        .rept 509
                .quad 0
        .endr
        .quad early_pd + 0x03
        .quad 0
early_pd:
        VALUE = 0x00000000
        .rept 512
                .quad VALUE + 0x83
                VALUE = VALUE + 0x00200000
        .endr

.section .bss
stack_bot:
        .skip 0x1000
stack:
