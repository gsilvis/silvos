.GLOBAL thread_start
thread_start:
        iretq

.GLOBAL schedule
schedule:
        push %rax
        push %rbx
        push %rcx
        push %rdx
        push %rbp
        push %rsi
        push %rdi
        push %r8
        push %r9
        push %r10
        push %r11
        push %r12
        push %r13
        push %r14
        push %r15

        mov %rsp,(schedule_rsp)
        call schedule_helper
        mov (schedule_rsp),%rsp
        mov (schedule_pt),%rax
        mov %rax,%cr3

        pop %r15
        pop %r14
        pop %r13
        pop %r12
        pop %r11
        pop %r10
        pop %r9
        pop %r8
        pop %rdi
        pop %rsi
        pop %rbp
        pop %rdx
        pop %rcx
        pop %rbx
        pop %rax

        ret
