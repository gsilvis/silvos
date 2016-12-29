KERNEL_OBJS :=  \
	start-asm.o \
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
	palloc.o \
	loader.o \
	com.o \
	acpi.o \
	hpet.o

TEST_PROGS := \
	test-debug \
	test-memory \
	test-float-fib \
	test-yield \
	test-exit

USERLAND_PROGS := \
	print-a \
	print-b \
	calc \
	$(TEST_PROGS)

KERN_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -O2 -g -mcmodel=kernel
KERN_LDFLAGS := -nostdlib -lgcc -Wl,-z,max-page-size=0x1000 -mcmodel=kernel

BOOTLOADER_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -O2 -g
BOOTLOADER_LDFLAGS := -nostdlib -Wl,--no-warn-mismatch -Wl,-z,max-page-size=0x1000

USER_CFLAGS := -ffreestanding -Wall -Wextra -Wno-main -std=c99 -O2
USER_LDFLAGS := -nostdlib -lgcc

CC := x86_64-elf-gcc
AS := x86_64-elf-as
OBJCOPY := x86_64-elf-objcopy

# Primary targets

all: bootloader.multiboot george.multiboot temp_drive \
	$(patsubst %, userland/%.bin, $(USERLAND_PROGS))

george.multiboot: $(patsubst %, kernel/%, $(KERNEL_OBJS))
	$(CC) -T kernel/kernel.ld $^ -o $@ $(KERN_LDFLAGS)

# Kernel Objects

kernel/%.o: kernel/%.c
	$(CC) -c $^ -o $@ $(KERN_CFLAGS)

kernel/%-asm.o: kernel/%-asm.s
	$(AS) $^ -o $@

# Bootloader

bootloader.multiboot: bootloader/start32.o bootloader/start64.o bootloader/start-asm.o
	$(CC) -T bootloader/bootloader.ld $^ -o $@ $(BOOTLOADER_LDFLAGS)

bootloader/start32.o: bootloader/start32.c
	$(CC) -m32 -c $^ -o $@ $(BOOTLOADER_CFLAGS)

bootloader/start64.o: bootloader/start64.c
	$(CC) -c $^ -o $@ $(BOOTLOADER_CFLAGS)

bootloader/start-asm.o: bootloader/start-asm.s
	$(AS) $^ -o $@

# User Objects

userland/%/main.o: userland/%/main.c
	$(CC) -c $^ -o $@ $(USER_CFLAGS) -Iuserland

userland/%.bin: userland/%/main.o userland/startup.o
	$(CC) $^ -o $@ $(USER_LDFLAGS)

## Special User Objects

userland/startup.o: userland/startup.s
	$(AS) $^ -o $@

# Convenience

george.disk: george.multiboot menu.lst
	./script/MKDISK.sh

temp_drive:
	dd if=/dev/zero of=$@ bs=512 count=16

# Testing

test: $(patsubst %, test/%, $(TEST_PROGS))

userland/%/output.txt: userland/%.bin george.multiboot bootloader.multiboot
	qemu-system-x86_64 -kernel bootloader.multiboot \
		-initrd george.multiboot,$(word 1,$^) \
		-serial file:$@ \
		-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
		-nographic \
		-display none \
		| true

test/%: userland/%/expected.txt userland/%/output.txt
	diff $^


clean:
	rm -f kernel/*.o george.multiboot george.disk userland/*.bin userland/*.o userland/*/*.o temp_drive bootloader.multiboot bootloader/*.o userland/*/output.txt
