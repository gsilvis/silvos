        .section .multiboot
        .align 4

        .long 0x1BADB002
        .long 0x00000003
        .long 0xE4524FFB

        .section .bss
        .align 16
stack_bot:
        .skip 0x1000
stack:

        .section .text
        .global _start
_start:
        movl $stack, %esp
        pushl %ebx /* Multiboot struct */
        pushl %eax /* Multiboot magic */
        call enter_long_mode
        ljmp $0x40,$kernel_main

.GLOBAL initialize_segment_selectors
initialize_segment_selectors:
        ljmp $0x30,$L1
L1:
        mov $0x10,%ax
        mov %ax,%ds
        mov %ax,%ss
        mov %ax,%es
        mov %ax,%fs
        mov %ax,%gs
        ret

.GLOBAL enable_ia32e
enable_ia32e:
        push %ecx
        mov $0xC0000080,%ecx
        rdmsr
        or $0x00000100,%eax
        wrmsr
        pop %ecx
        ret
