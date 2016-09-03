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
        .type _start, @function
_start:
        movl $stack, %esp
        pushl %ebx /* Multiboot struct */
        pushl %eax /* Multiboot magic */
        call kernel_main
