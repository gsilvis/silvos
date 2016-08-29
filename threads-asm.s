.GLOBAL thread_start
thread_start:
        iret

.GLOBAL schedule
schedule:
        mov %esp,(schedule_esp)
        call schedule_helper
        mov (schedule_esp),%esp
        mov (schedule_pt),%eax
        mov %eax,%cr3
        ret
