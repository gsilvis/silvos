.GLOBAL _start
_start:
	call main

	/* call exit */
	mov $0x02, %rax
	int $0x36
