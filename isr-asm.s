.GLOBAL yield_isr
.GLOBAL thread_start
yield_isr:
        pusha
L1:
        mov %esp,(schedule_esp)
        call schedule
        mov (schedule_esp),%esp
        mov (schedule_pt),%eax
        mov %eax,%cr3
        popa
        iret

.GLOBAL kbd_isr
kbd_isr:
        pusha
        call eoi
        call read_key
        test %eax,%eax
        jnz L1
        popa
        iret

.GLOBAL timer_isr
timer_isr:
        pusha
        call eoi
        jmp L1

.GLOBAL putch_isr
putch_isr:
        pusha
        push %eax
        call putc
        pop %eax
        popa
        iret

.GLOBAL exit_isr
exit_isr:
        pusha
        call thread_exit
        jmp L1
