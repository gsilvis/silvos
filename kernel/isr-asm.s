
/* Hardware interrupts must push all caller-save registers.   Syscall handlers
 * don't need to do anything.  (Callee-save registers are saved later.  For
 * hardware interrupts, this means all registers are saved.  The syscall ABI
 * lets the kernel clobber caller-save registers.)  Be careful about using any
 * registers in these routines. */

.macro push_caller_save_reg
	push %rax
	push %rcx
	push %rdx
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
.endm

.macro pop_caller_save_reg
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rdx
	pop %rcx
	pop %rax
.endm

.macro clear_caller_saved_reg_except_rax
	xor %r11, %r11
	xor %r10, %r10
	xor %r9, %r9
	xor %r8, %r8
	xor %rdi, %rdi
	xor %rsi, %rsi
	xor %rdx, %rdx
	xor %rcx, %rcx
.endm

/* Syscalls */

/* Note: the syscall ABI is:
 *
 *  REG | INPUT | OUTPUT
 *  --------------------
 *  RAX | SYSNO | RETURN
 *  RBX | ARG1  | SAVED
 *  RCX | ARG2  | UNDEF
 *  RDX | ----- | UNDEF
 *  RBP | ----- | SAVED
 *  RSI | ----- | UNDEF
 *  RDI | ----- | UNDEF
 *  RSP | ----- | SAVED
 *  R8  | ----- | UNDEF
 *  R9  | ----- | UNDEF
 *  R10 | ----- | UNDEF
 *  R11 | ----- | UNDEF
 *  R12 | ----- | SAVED
 *  R13 | ----- | SAVED
 *  R14 | ----- | SAVED
 *  R15 | ----- | SAVED
 *
 * This matches the SYSV x86_64 ABI for caller/callee saved registers,
 * but not for arguments */

.GLOBAL syscall_isr
syscall_isr:
	movq (syscall_defns_len), %r8
	cmpq %r8, %rax
	jae invalid_syscall

	mov $syscall_defns, %r8
	mov (%r8, %rax, 8), %rax

	/* Translate syscall arguments into appropriate regs for handler */
	mov %rbx,%rdi
	mov %rcx,%rsi
	call *%rax

	/* Clear registers to prevent information leakage from kernel mode
	 * to usermode. For *callee* saved registers, it would be an ABI
	 * to not have preserved them, so we only need to clobber caller
	 * saved registers */
	clear_caller_saved_reg_except_rax
	iretq

invalid_syscall:
	mov $-1, %rax
	/* Note: caller saved registers don't need to be clobbered */
	iretq

/* Interrupts */

.GLOBAL kbd_isr
kbd_isr:
	push_caller_save_reg
	call master_eoi
	call read_key
	pop_caller_save_reg
	iretq

.GLOBAL timer_isr
timer_isr:
	push_caller_save_reg
	call master_eoi
	call yield
	pop_caller_save_reg
	iretq

.GLOBAL rtc_isr
rtc_isr:
	push_caller_save_reg
	call slave_eoi
	call hpet_sleepers_awake
	pop_caller_save_reg
	iretq

.GLOBAL ac97_isr
ac97_isr:
        push_caller_save_reg
        call ac97_handle_interrupt
        call slave_eoi
        pop_caller_save_reg
        iretq

/* Faults */

.GLOBAL nm_isr
nm_isr:
	push_caller_save_reg
	call fpu_activate
	pop_caller_save_reg
	iretq

.GLOBAL fault_isr
fault_isr:
	push_caller_save_reg
	call thread_exit_fault

.GLOBAL df_isr
df_isr:
	hlt
	jmp df_isr

.GLOBAL pf_isr
pf_isr:
	push_caller_save_reg
	mov %cr2,%rdi
	call pagefault_handler
	pop_caller_save_reg
	iretq
