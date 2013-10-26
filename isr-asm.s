.GLOBAL yield_isr
yield_isr:
        pusha
L1:
        push %esp
        call schedule
        mov %eax,%esp /* Switch stacks here! */
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
