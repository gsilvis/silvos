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

.GLOBAL thread_start_within_old_vm_space
thread_start_within_old_vm_space:
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

/* fork must be written in assembly, so we know that no stack pointers are put
 * in registers or on the stack---those will break when we move to a new stack.
 */

.GLOBAL fork
fork:
	call fork_entry_point
	ret

/* fork_entry_point and schedule's stack frames must look the same: when
 * forking, the stack is copied while in fork_entry_point, but the child
 * resumes into schedule instead.  Both parent and child return into 'fork',
 * which then returns to userland. */

.GLOBAL fork_entry_point
fork_entry_point:
	push_callee_save_reg
	mov %rsp,%rdi
	call clone_thread
	pop_callee_save_reg
	ret

.GLOBAL com_print_backtrace
com_print_backtrace:
	mov %rsp,%rdi
	add $0x8,%rdi
	mov (%rsp),%rsi
	mov %rbp,%rdx
	jmp gen_backtrace
