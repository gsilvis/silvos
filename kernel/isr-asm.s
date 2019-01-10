/* All interrupts push all registers, then copy all of them into
 * 'current_tcb->saved_registers', of type 'struct all_registers' (defined in
 * thread.h) .  They then eventually call a method that does not return.
 *
 * With few exceptions, methods called from this file (and methods in general)
 * should either /always/ return or /never/ return.  Functions that sometimes
 * return may cause bugs where the code after their call is only sometimes
 * called.
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
	call syscall_handler  /* never returns */

/* Interrupts */

.GLOBAL kbd_isr
kbd_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call master_eoi
	call read_key
	call return_to_current_thread  /* never returns */

.GLOBAL timer_isr
timer_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call master_eoi
	call yield  /* never returns */

.GLOBAL rtc_isr
rtc_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call slave_eoi
	call hpet_sleepers_awake
	call return_to_current_thread  /* never returns */

/* Faults */

.GLOBAL nm_isr
nm_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call fpu_activate  /* never returns */

.GLOBAL fault_isr
fault_isr:
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call thread_exit_fault  /* never returns */

.GLOBAL df_isr
df_isr:
	hlt
	jmp df_isr

/* pf_isr is special because it can happen in two ways.  First we handle the
 * possibility that we crashed during 'copy_from_user' or 'copy_to_user'; if
 * that was the cause, then 'pagefault_handler_copy' will not return.  Under no
 * circumstances will 'pagefault_handler_copy' use the saved registers or
 * switch threads; that is why it safe to defer 'save_thread_registers' until
 * after it.  Even more so, if this was a copy to/from user pagefault, we
 * actively do not want to save the registers.
 */
.GLOBAL pf_isr
pf_isr:
	push_general_purpose_reg
	call pagefault_handler_copy  /* sometimes returns */
	mov %rsp,%rdi
	call save_thread_registers
	mov %cr2,%rdi
	call pagefault_handler_user  /* never returns */


/* One special method: enter_userspace is the only way to return from an ISR.
 *
 * The caller passes a pointer to a 'struct all_registers', which has exactly
 * the structure we would like our stack to have as we "return" to userspace.
 * So, move our stack there, pop our registers, and return!
 *
 * Frequently the 'struct all_registers' is actually located in the
 * 'saved_register' field of a thread's TCB.  Moving our stack there is a
 * little bit alarming, but all we do is pop and iretq, so it's fine.
 */
.GLOBAL enter_userspace
enter_userspace:
	mov %rdi,%rsp
	pop_general_purpose_reg
	iretq
