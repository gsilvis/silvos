.GLOBAL initialize_segment_selectors

initialize_segment_selectors:
    ljmp $0x08,$L1
L1:
    mov $0x10,%ax
    mov %ax,%ds
    mov %ax,%ss
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    ret
