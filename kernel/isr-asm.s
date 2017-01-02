
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

/* Syscalls */

.GLOBAL syscall_isr
syscall_isr:
	mov %rbx,%rdi
	mov %rcx,%rsi
	mov $syscall_defns, %r8
	mov (%r8, %rax, 8), %rax
	call *%rax
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
	call thread_exit

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
