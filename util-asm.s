.GLOBAL delay
delay:
        mov 4(%esp),%ecx
L1:
        loop L1
        ret
