OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

KERNEL_BASE = 0xFFFFFFFF80000000;

SECTIONS
{
    . = 1M + SIZEOF_HEADERS;

    .init : AT(ADDR(.init)) {
        *(.multiboot)
    }

    .inittext : AT(ADDR(.inittext)) {
        *(.inittext)
    }

    .initdata : AT(ADDR(.initdata)) {
        *(.initdata)
    }

    .initpt : AT(ADDR(.initpt)) {
        *(.initpt)
    }

    . += KERNEL_BASE;

    .text : AT(ADDR(.text) - KERNEL_BASE) {
        *(.text)
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_BASE) {
        *(.rodata)
    }

    .eh_frame : AT(ADDR(.eh_frame) - KERNEL_BASE) {
      _eh_frame_start = .;
      *(.eh_frame)
      _eh_frame_end = .;
    }

    .data : AT(ADDR(.data) - KERNEL_BASE) {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - KERNEL_BASE) {
        *(COMMON)
        *(.bss)
    }
    _end = .;
}
