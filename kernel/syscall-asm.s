.section .rodata
.GLOBAL syscall_defns
syscall_defns:
	.quad yield
	.quad putc
	.quad thread_exit
	.quad getch
	.quad read_sector
	.quad write_sector
	.quad palloc
	.quad pfree
	.quad com_debug_thread
	.quad hpet_nanosleep
	.quad fork

.GLOBAL syscall_defns_len
syscall_defns_len:
	.quad (. - syscall_defns)>>3
