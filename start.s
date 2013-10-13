        .text
        .global _start
_start:
        jmp multiboot_start
        .align 4
multiboot_header:
        .long 0x1BADB002
        .long 0x00000003
        .long 0xE4524FFB
multiboot_start:        
        movl $stack, %esp
        pushl %ebx /* Multiboot struct */
        pushl %eax /* Multiboot magic */
        call kernel_main
spin:
        hlt
        jmp spin

stack_bot:
        .skip 0x1000
stack:  
