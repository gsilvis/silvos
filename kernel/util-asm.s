
.GLOBAL setjmp
setjmp:
	movq (%rsp),%rax
	mov %rax,0x00(%rdi)
	mov %rsp,0x08(%rdi)
	mov %rbx,0x10(%rdi)
	mov %rbp,0x18(%rdi)
	mov %r12,0x20(%rdi)
	mov %r13,0x28(%rdi)
	mov %r14,0x30(%rdi)
	mov %r15,0x38(%rdi)
	mov $0,%rax
	ret

.GLOBAL longjmp
longjmp:
	mov 0x38(%rdi),%r15
	mov 0x30(%rdi),%r14
	mov 0x28(%rdi),%r13
	mov 0x20(%rdi),%r12
	mov 0x18(%rdi),%rbp
	mov 0x10(%rdi),%rbx
	mov 0x08(%rdi),%rsp
	mov 0x00(%rdi),%rax
	mov %rax,(%rsp)
	mov %rsi,%rax
	ret
