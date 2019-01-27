/* All interrupts push all registers, then copy all of them into
 * 'current_tcb->saved_registers', of type 'struct all_registers' (defined in
 * thread.h) .  They then eventually call a method that does not return.
 *
 * With few exceptions, methods called from this file (and methods in general)
 * should either /always/ return or /never/ return.  Functions that sometimes
 * return may cause bugs where the code after their call is only sometimes
 * called.
 */

/* Helpers for debugging.  Most IRSs also want to have '.cfi_signal_frame';
 * this indicates that the function is "signal-like", in that it interrupted
 * code and was not caused by a call-like instruction.  Specifically, it makes
 * it so that the RIP might be the first instruction in the parent function,
 * and will not be one off the end. */
.macro isr_prologue
	.cfi_startproc
	.cfi_def_cfa %rsp,0x20
	.cfi_offset %rip,-0x20
	.cfi_offset %cs,-0x18
	.cfi_offset %eflags,-0x10
	.cfi_offset %rsp,-0x08
	.cfi_offset %ss,0x00
.endm

.macro isr_epilogue
	.cfi_endproc
.endm

/* Some Intel exceptions push status codes and some don't.  To make our 'all
 * registers' structs have a consistent format, it's easiest to leave a space
 * for the code either way.  Always call one of these two macros! */
.macro push_fake_status_code
	pushq $0
	.cfi_adjust_cfa_offset 0x08
.endm

.macro intel_pushes_status_code
	.cfi_adjust_cfa_offset 0x08
.endm

/* This must stay in sync with 'struct all_registers' in threads.h */
.macro push_general_purpose_reg
	push %rax
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rax,0x00
	push %rbx
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rbx,0x00
	push %rcx
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rcx,0x00
	push %rdx
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rdx,0x00
	push %rbp
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rbp,0x00
	push %rsi
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rsi,0x00
	push %rdi
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %rdi,0x00
	push %r8
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r8,0x00
	push %r9
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r9,0x00
	push %r10
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r10,0x00
	push %r11
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r11,0x00
	push %r12
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r12,0x00
	push %r13
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r13,0x00
	push %r14
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r14,0x00
	push %r15
	.cfi_adjust_cfa_offset 0x08
	.cfi_rel_offset %r15,0x00
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

/* The syscall ABI for syscalls other than sendrecv() is:
 *
 *  REG | INPUT | OUTPUT
 *  --------------------
 *  RAX | SYSNO | RETURN
 *  RBX | ARG1  | PRESERVED
 *  RCX | ARG2  | PRESERVED
 *
 * All other registers are unused and preserved.
 *
 * For call(), respond(), and fork_daemon() it is instead:
 *
 *  REG | INPUT | OUTPUT
 *  --------------------
 *  RAX | SYSNO | RETURN (status)
 *  RBX | ADDR  | RETURN
 *  RCX | ARG1  | RETURN
 *  RDX | ARG2  | RETURN
 *
 * (For fork_daemon in particular, 'ADDR' is ignored.  The same return ABI is
 * used in the old thread and in the new thread.)
 *
 * All other registers are still unused and preserved.  The RETURN register
 * meanings correspond to the inputs, eg RBX is the address that a message came
 * from.
 */

.GLOBAL syscall_isr
syscall_isr:
	isr_prologue
	push_fake_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call syscall_handler  /* never returns */
	isr_epilogue

/* Interrupts */

.GLOBAL kbd_isr
kbd_isr:
	isr_prologue
	.cfi_signal_frame
	push_fake_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call apic_eoi
	call read_key
	call return_to_current_thread  /* never returns */
	isr_epilogue

.GLOBAL timer_isr
timer_isr:
	isr_prologue
	.cfi_signal_frame
	push_fake_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call apic_eoi
	call yield  /* never returns */
	isr_epilogue

.GLOBAL rtc_isr
rtc_isr:
	isr_prologue
	.cfi_signal_frame
	push_fake_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call apic_eoi
	call hpet_sleepers_awake
	call return_to_current_thread  /* never returns */
	isr_epilogue

.GLOBAL ide_isr
ide_isr:
	isr_prologue
	.cfi_signal_frame
	push_fake_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call apic_eoi
	call ide_handler
	call return_to_current_thread  /* never returns */
	isr_epilogue

/* Faults */

.GLOBAL nm_isr
nm_isr:
	isr_prologue
	.cfi_signal_frame
	push_fake_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call fpu_activate  /* never returns */
	isr_epilogue

.GLOBAL fault_isr
fault_isr:
	isr_prologue
	.cfi_signal_frame
	intel_pushes_status_code
	push_general_purpose_reg
	mov %rsp,%rdi
	call save_thread_registers
	call thread_exit_fault  /* never returns */
	isr_epilogue

.GLOBAL df_isr
df_isr:
	isr_prologue
	.cfi_signal_frame
	intel_pushes_status_code  /* even though it's always 0... */
	hlt
	jmp df_isr
	isr_epilogue

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
	isr_prologue
	.cfi_signal_frame
	intel_pushes_status_code
	push_general_purpose_reg
	call pagefault_handler_copy  /* sometimes returns */
	mov %rsp,%rdi
	call save_thread_registers
	mov %cr2,%rdi
	call pagefault_handler_user  /* never returns */
	isr_epilogue


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
	add $8,%rsp  /* Skip over the status code. */
	iretq

/* One more special method: IDLE.  This is written in assembly to represent two
 * facts:  first, one should not call it lightly; always execute it by IRETQing
 * to it, so you're on a different stack.  Second, please don't try and unwind
 * through it; it won't go well, because we got to it by returning, not by
 * calling. */
.GLOBAL idle
idle:
	hlt
	jmp idle
