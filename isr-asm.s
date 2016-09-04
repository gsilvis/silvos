
/* Hardware interrupts must push all caller-save registers.  Syscall handlers
 * don't need to do anything. */

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
