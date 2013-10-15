.GLOBAL yield_isr
yield_isr:
        pusha
        push %esp
        call schedule
        add $4,%esp
        mov %eax,%esp /* Switch stacks here! */
        popa
        iret
