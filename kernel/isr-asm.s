
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

.GLOBAL yield_isr
yield_isr:
        call schedule
        iretq

.GLOBAL putch_isr
putch_isr:
        mov %rax,%rdi
        call putc
        iretq

.GLOBAL exit_isr
exit_isr:
        call thread_exit
        call schedule

.GLOBAL getch_isr
getch_isr:
        call getch
        iretq

.GLOBAL read_isr
read_isr:
	mov %rax,%rdi
	mov %rbx,%rsi
	call read_sector
	iretq

.GLOBAL write_isr
write_isr:
	mov %rax,%rdi
	mov %rbx,%rsi
	call write_sector
	iretq

.GLOBAL palloc_isr
palloc_isr:
	mov %rax,%rdi
	call palloc
	iretq

.GLOBAL pfree_isr
pfree_isr:
	mov %rax,%rdi
	call pfree
	iretq

.GLOBAL debug_isr
debug_isr:
	mov %rax,%rdi
	mov %rbx,%rsi
	call com_debug_thread
	iretq

/* Interrupts */

.GLOBAL kbd_isr
kbd_isr:
        push_caller_save_reg
        call eoi
        call read_key
        pop_caller_save_reg
        iretq

.GLOBAL timer_isr
timer_isr:
        push_caller_save_reg
        call eoi
        call schedule
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
        call schedule

.GLOBAL df_isr
df_isr:
        hlt
        jmp df_isr

.GLOBAL pf_isr
pf_isr:
        push_caller_save_reg
        call pagefault_handler
        pop_caller_save_reg
        iretq
