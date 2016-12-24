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
	.code32
.GLOBAL _start
_start:
	movl $stack,%esp

	/* Save multiboot information */
	pushl %ebx
	pushl %eax
	call check_magic
	call setup_gdt

	/* initialize segment selectors */
	ljmp $0x10,$1f
1:
	mov $0x08,%ax
	mov %ax,%ds
	mov %ax,%ss
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

	call init_page_table

	/* Enable PAE */
        /* Also enable SMEP */
	movl %cr4,%edi
	orl $0x00100020,%edi
	movl %edi,%cr4

	/* Load page table */
	movl $bad_pml4,%edi
	movl %edi,%cr3

	/* Enable ia32e (in 32-bit compatibility mode) */
	/* Also enable NX bit */
	movl $0xC0000080,%ecx
	rdmsr
	orl $0x00000900,%eax
	wrmsr

	/* Enable paging */
	movl %cr0,%edi
	orl $0x80000000,%edi
	movl %edi,%cr0

	/* Recover multiboot information */
	popl %eax
	popl %ebx

	/* Enter long mode */
	ljmp $0x18,$_start64

	.code64
.GLOBAL _start64
_start64:
	/* Save multiboot information, again */
	pushq %rbx
	pushq %rax

	movq %rbx,%rdi
	call relocate_kernel
	movq %rax,%rdi

	/* Recover multiboot information, again */
	popq %rax
	popq %rbx

	/* Return to the (mapped) kernel */
	pushq %rdi
	retq
