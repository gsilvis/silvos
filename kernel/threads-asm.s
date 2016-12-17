.GLOBAL thread_start
thread_start:
        iretq

.GLOBAL user_thread_start
user_thread_start:
        call user_thread_launch
        iretq

.GLOBAL schedule
schedule:
        /* Push callee-save registers */
        push %rbx
        push %rbp
        push %r12
        push %r13
        push %r14
        push %r15

        mov %rsp,(schedule_rsp)
        call schedule_helper
        mov (schedule_rsp),%rsp
        mov (schedule_pt),%rax
        mov %rax,%cr3

        /* Pop callee-save registers */
        pop %r15
        pop %r14
        pop %r13
        pop %r12
        pop %rbp
        pop %rbx

        ret
