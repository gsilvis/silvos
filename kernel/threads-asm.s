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
	call thread_launch
        iretq

.GLOBAL schedule
schedule:
	push_callee_save_reg
	mov %rsp,%rdi
	call start_context_switch
	mov %rax,%rsp  /* Switch stacks here! */
	call finish_context_switch
	pop_callee_save_reg
	ret

.GLOBAL fork_entry_point
fork_entry_point:
	push_callee_save_reg
	/* We will copy everything above us to the child's new kernel stack.
	 * This means the child will exit insert_pt (in schedule) with an exact
	 * copy of the current stack; above us is all callee-save registers
	 * followed by the return address into fork, then the syscall stack. */
	mov %rsp,%rdi
	call clone_thread
	pop_callee_save_reg
	ret
