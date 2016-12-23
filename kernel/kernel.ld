OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFFC0200000;

    _multiboot_header = .;
    .multiboot BLOCK(4K) : ALIGN(4K)
    {
        LONG(0x1BADB002)
	LONG(0x00008003)
	LONG(0x00000000 - 0x1BADB002 - 0x00008003)
	LONG(_multiboot_header - 0xFFFFFFFFC0000000)
	LONG(0x00200000)
	LONG(_data_end - 0xFFFFFFFFC0000000)
	LONG(_end - 0xFFFFFFFFC0000000)
	LONG(_start - 0xFFFFFFFFC0000000)
    }

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }
    _data_end = .;

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
    _end = .;
}
