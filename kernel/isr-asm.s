/* All interrupts push all registers, then save a pointer to them into
 * 'current_tcb->saved_registers', of type 'struct all_registers *' (defined in
 * thread.h) .
 */

/* This must stay in sync with 'struct all_registers' in threads.h */
.macro push_general_purpose_reg
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rbp
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15
.endm

.macro pop_general_purpose_reg
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
.endm

/* Syscalls */

/* The syscall ABI is:
 *
 *  REG | INPUT | OUTPUT
 *  --------------------
 *  RAX | SYSNO | RETURN
 *  RBX | ARG1  | PRESERVED
 *  RCX | ARG2  | PRESERVED
 *
 * All other registers are unused and preserved.
 */

.GLOBAL syscall_isr
syscall_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call get_syscall_handler
	call *%rax
	pop_general_purpose_reg
	iretq

/* Interrupts */

.GLOBAL kbd_isr
kbd_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call master_eoi
	call read_key
	pop_general_purpose_reg
	iretq

.GLOBAL timer_isr
timer_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call master_eoi
	call yield
	pop_general_purpose_reg
	iretq

.GLOBAL rtc_isr
rtc_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call slave_eoi
	call hpet_sleepers_awake
	pop_general_purpose_reg
	iretq

/* Faults */

.GLOBAL nm_isr
nm_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call fpu_activate
	pop_general_purpose_reg
	iretq

.GLOBAL fault_isr
fault_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call thread_exit_fault

.GLOBAL df_isr
df_isr:
	hlt
	jmp df_isr

/* pf_isr is special because it can happen in two ways.  First we handle the
 * possibility that we crashed during 'copy_from_user' or 'copy_to_user'; if
 * that was the cause, then 'pagefault_handler_copy' will not return.  Under no
 * circumstances will 'pagefault_handler_copy' use the saved registers.
 */

.GLOBAL pf_isr
pf_isr:
	push_general_purpose_reg
	call pagefault_handler_copy
	mov %rsp,%rdi
	call save_thread_registers
	mov %cr2,%rdi
	call pagefault_handler_user
