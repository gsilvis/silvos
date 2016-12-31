.section .rodata
.GLOBAL syscall_defns
syscall_defns:
	.quad schedule
	.quad putc
	.quad thread_exit_schedule
	.quad getch
	.quad read_sector
	.quad write_sector
	.quad palloc
	.quad pfree
	.quad com_debug_thread
	.quad hpet_nanosleep
	.quad fork
