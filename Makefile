KERNEL_OBJS :=  \
	start-asm.o \
	start.o \
	george.o \
	threads.o \
	threads-asm.o \
	vga.o \
	util.o \
	idt.o \
	isr-asm.o \
	pic.o \
	pit.o \
	util-asm.o \
	gdt.o \
	page.o \
	alloc.o \
	kbd.o \
	fpu.o \
	pagefault.o \
	pci.o \
	ide.o \
	palloc.o

KERN_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -O2 -g
KERN_CFLAGS_32 := $(KERN_CFLAGS) -m32
KERN_LDFLAGS := -nostdlib -lgcc -Wl,-z,max-page-size=0x1000

USER_CFLAGS := -ffreestanding -Wall -Wextra -Wno-main -std=c99 -O2 -fpie
USER_LDFLAGS := -nostdlib -lgcc

all: george.multiboot temp_drive

george.multiboot: $(KERNEL_OBJS)
	x86_64-elf-gcc -T kernel.ld $^ -o $@ $(KERN_LDFLAGS)

start32-asm.o: start32-asm.s
	x86_64-elf-as --32 $^ -o $@

start32.o: start32.c
	x86_64-elf-gcc -c $^ -o $@ $(KERN_CFLAGS_32)

start-asm.o: start32-asm.o
	x86_64-elf-objcopy $^ $@ -I elf32-i386 -O elf64-x86-64

start.o: start32.o
	x86_64-elf-objcopy $^ $@ -I elf32-i386 -O elf64-x86-64

%-asm.o: %-asm.s
	x86_64-elf-as $^ -o $@

george.o: george.c userland/print-a-include.h userland/print-b-include.h userland/calc-include.h
	x86_64-elf-gcc -c $< -o $@ $(KERN_CFLAGS)

%.o: %.c
	x86_64-elf-gcc -c $^ -o $@ $(KERN_CFLAGS)

userland/%.o: userland/%.c
	x86_64-elf-gcc -c $^ -o $@ $(USER_CFLAGS)

userland/startup.o: userland/startup.s
	x86_64-elf-as $^ -o $@

userland/%.bin: userland/%.o userland/startup.o
	x86_64-elf-gcc -T userland/linker-script $< -o $@ $(USER_LDFLAGS)

userland/%-include.h: userland/%.bin
	xxd -i $^ > $@

george.disk: george.multiboot menu.lst
	./script/MKDISK.sh

temp_drive:
	dd if=/dev/zero of=$@ bs=512 count=16

clean:
	rm -f *.o george.multiboot george.disk userland/*.bin userland/*.o userland/*-include.h temp_drive
