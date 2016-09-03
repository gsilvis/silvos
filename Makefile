
all: george.multiboot

george.multiboot: george.o start-asm.o threads.o vga.o util.o idt.o isr.o isr-asm.o pic.o pit.o util-asm.o gdt.o gdt-asm.o page.o alloc.o threads-asm.o kbd.o
	i686-elf-gcc -T kernel.ld $^ -o $@ -nostdlib -lgcc

%-asm.o: %-asm.s
	i686-elf-as $^ -o $@

george.o: george.c userland/print-a-include.h userland/print-b-include.h userland/calc-include.h
	i686-elf-gcc -c $< -o $@ -ffreestanding -Wall -Wextra -std=c99 -O2 -g

%.o: %.c
	i686-elf-gcc -c $< -o $@ -ffreestanding -Wall -Wextra -std=c99 -O2 -g

userland/%.o: userland/%.c
	i686-elf-gcc -c $^ -o $@ -ffreestanding -Wall -Wextra -Wno-main -std=c99 -O2

userland/startup.o: userland/startup.s
	i686-elf-as $^ -o $@

userland/%.bin: userland/%.o userland/startup.o
	i686-elf-gcc -T userland/linker-script $< -o $@ -nostdlib -lgcc

userland/%-include.h: userland/%.bin
	xxd -i $^ > $@

george.disk: george.multiboot menu.lst
	./MKDISK.sh

clean:
	rm -f *.o george.multiboot george.disk userland/*.bin userland/*.o userland/*-include.h
