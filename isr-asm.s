.GLOBAL yield_isr
yield_isr:
        pusha
        call schedule
        popa
        iret

.GLOBAL kbd_isr
kbd_isr:
        pusha
        call eoi
        call read_key
        popa
        iret

.GLOBAL timer_isr
timer_isr:
        pusha
        call eoi
        call schedule
        popa
        iret

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
        call schedule

.GLOBAL getch_isr
getch_isr:
        pusha
        call getch
        mov %eax,28(%esp)
        popa
        iret
