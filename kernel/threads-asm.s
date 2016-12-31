.macro push_callee_save_reg
        push %rbx
        push %rbp
        push %r12
        push %r13
        push %r14
        push %r15
.endm

.macro pop_callee_save_reg
        pop %r15
        pop %r14
        pop %r13
        pop %r12
        pop %rbp
        pop %rbx
.endm

.GLOBAL thread_start
thread_start:
        iretq

.GLOBAL user_thread_start
user_thread_start:
        call user_thread_launch
        iretq

.GLOBAL schedule
schedule:
        push_callee_save_reg
        mov %rsp,(schedule_rsp)
        call schedule_helper
        mov (schedule_rsp),%rsp
        mov (schedule_pt),%rdi
        call insert_pt
        pop_callee_save_reg
        ret

.section .bss
.comm fork_ret,4,4

.text

.GLOBAL fork
fork:
	call fork_entry_point
	/* Both parent and child return here. The parent returns immediately,
	 * so fork_ret will contain 0 for the child. */
	mov (fork_ret),%edi
	movq $0,(fork_ret)
	call finish_fork
	ret

fork_entry_point:
	push_callee_save_reg
	/* We will copy everything above us to the child's new kernel stack.
	 * This means the child will exit insert_pt (in schedule) with an exact
	 * copy of the current stack; above us is all callee-save registers
	 * followed by the return address into fork, then the syscall stack. */
	mov %rsp,%rdi
	call clone_thread
	mov %eax,(fork_ret)
	pop_callee_save_reg
	ret

finish_fork:
	testl	%edi, %edi
	pushq	%rbx
	movl	%edi, %ebx
	/* if the fork_ret was not zero, we are in the parent and can just return */
	jne     finish_fork_child
	call	apply_pagemap
finish_fork_child:
	movl	%ebx, %eax
	popq	%rbx
	ret
