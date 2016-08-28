.GLOBAL yield_isr
.GLOBAL thread_start
yield_isr:
        pusha
L1:
;        mov $0x10,%ax
;        mov %ax,%ds
;        mov %ax,%es
;        mov %ax,%fs
;        mov %ax,%gs
        call schedule
thread_start:
;        mov $0x23,%ax
;        mov %ax,%ds
;        mov %ax,%es
;        mov %ax,%fs
;        mov %ax,%gs
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
