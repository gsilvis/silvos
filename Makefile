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

USERLAND_PROGS := \
	print-a \
	print-b \
	calc

KERN_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -O2 -g
KERN_CFLAGS_32 := $(KERN_CFLAGS) -m32
KERN_LDFLAGS := -nostdlib -lgcc -Wl,-z,max-page-size=0x1000

USER_CFLAGS := -ffreestanding -Wall -Wextra -Wno-main -std=c99 -O2 -fpie
USER_LDFLAGS := -nostdlib -lgcc

CC := x86_64-elf-gcc
AS := x86_64-elf-as
OBJCOPY := x86_64-elf-objcopy

# Primary targets

all: george.multiboot temp_drive

george.multiboot: $(patsubst %, kernel/%, $(KERNEL_OBJS))
	$(CC) -T kernel/kernel.ld $^ -o $@ $(KERN_LDFLAGS)

# Kernel Objects

kernel/%.o: kernel/%.c
	$(CC) -c $^ -o $@ $(KERN_CFLAGS)

kernel/%-asm.o: kernel/%-asm.s
	$(AS) $^ -o $@

## Special Kernel Objects

kernel/start32-asm.o: kernel/start32-asm.s
	$(AS) --32 $^ -o $@

kernel/start32.o: kernel/start32.c
	$(CC) -c $^ -o $@ $(KERN_CFLAGS_32)

kernel/start-asm.o: kernel/start32-asm.o
	$(OBJCOPY) $^ $@ -I elf32-i386 -O elf64-x86-64

kernel/start.o: kernel/start32.o
	$(OBJCOPY) $^ $@ -I elf32-i386 -O elf64-x86-64

kernel/george.o: kernel/george.c $(patsubst %, userland/%-include.h, $(USERLAND_PROGS))
	$(CC) -c $< -o $@ $(KERN_CFLAGS) -I.

# User Objects

userland/%/main.o: userland/%/main.c
	$(CC) -c $^ -o $@ $(USER_CFLAGS) -Iuserland

userland/%.bin: userland/%/main.o userland/startup.o
	$(CC) -T userland/linker-script $< -o $@ $(USER_LDFLAGS)

userland/%-include.h: userland/%.bin
	xxd -i $^ > $@

## Special User Objects

userland/startup.o: userland/startup.s
	$(AS) $^ -o $@

# Convenience

george.disk: george.multiboot menu.lst
	./script/MKDISK.sh

temp_drive:
	dd if=/dev/zero of=$@ bs=512 count=16

clean:
	rm -f kernel/*.o george.multiboot george.disk userland/*.bin userland/*.o userland/*/*.o userland/*-include.h temp_drive
