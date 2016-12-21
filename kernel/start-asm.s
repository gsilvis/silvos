	.section .bss
	.align 16
stack_bot:
	.skip 0x1000
stack:

	.section .text
.GLOBAL _start
_start:
	call insert_gdt
	pushq $0x40
	pushq $1f
	lretq
1:
	mov $0x10,%ax
	mov %ax,%ds
	mov %ax,%ss
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

	movq $stack,%rsp
	movq %rbx,%rdi
	call kernel_main

