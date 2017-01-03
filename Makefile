KERNEL_OBJS := \
	$(patsubst kernel/%.c, kernel/%.o, $(wildcard kernel/*.c)) \
	$(patsubst kernel/%.s, kernel/%.o, $(wildcard kernel/*.s))

TEST_PROGS := $(patsubst userland/%/expected.txt, %, $(wildcard userland/*/expected.txt))

USERLAND_PROGS := $(patsubst userland/%/main.c, %, $(wildcard userland/*/main.c))

# Ideally, the kernel should compile and pass all tests with all of these
#KERNEL_OPT := -O0
#KERNEL_OPT := -O1
KERNEL_OPT := -O2
#KERNEL_OPT := -O3
#KERNEL_OPT := -Os
KERNEL_OPT += -g
#KERNEL_OPT += -flto

KERN_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel $(KERNEL_OPT)
KERN_LDFLAGS := -nostdlib -lgcc -Wl,-z,max-page-size=0x1000 -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2 $(KERNEL_OPT)

BOOTLOADER_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -O2 -g
BOOTLOADER_LDFLAGS := -nostdlib -Wl,--no-warn-mismatch -Wl,-z,max-page-size=0x1000

USER_CFLAGS := -ffreestanding -Wall -Wextra -Wno-main -std=c99 -O2
USER_LDFLAGS := -nostdlib -lgcc

CC := x86_64-elf-gcc
AS := x86_64-elf-as
OBJCOPY := x86_64-elf-objcopy

# this instructs GNU make to use bash as our shell
SHELL = /bin/bash

# Primary targets

all: bootloader.multiboot george.multiboot temp_drive \
	$(patsubst %, userland/%.bin, $(USERLAND_PROGS))

george.multiboot: $(KERNEL_OBJS)
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

.PRECIOUS: $(patsubst %, userland/%/output.txt, $(TEST_PROGS))

userland/%/test_disk:
	dd if=/dev/zero of=$@ bs=512 count=16

## This rule enforces that coverage.log requires output.txt to be built
## But doesn't do anything else
userland/%/coverage.log: userland/%/output.txt

userland/%/output.txt: userland/%.bin george.multiboot bootloader.multiboot userland/%/test_disk
	qemu-system-x86_64 -kernel bootloader.multiboot \
		-d in_asm -D >( grep -E "^(Trace.*\[ffffffff|IN:|0xffffff)" | uniq > $(subst output.txt,coverage.log,$@)) \
		-initrd george.multiboot,$< \
		-serial file:$@ \
		-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
		-nographic \
		-display none \
		-m 128 \
		-hda $(subst output.txt,test_disk,$@) \
		| true

test/%: userland/%/expected.txt userland/%/output.txt
	diff $^


clean:
	rm -f \
		kernel/*.o \
		george.multiboot \
		george.disk \
		userland/*.bin \
		userland/*.o \
		userland/*/*.o \
		temp_drive \
		bootloader.multiboot \
		bootloader/*.o \
		userland/*/output.txt \
		userland/*/coverage.log \
		userland/*/test_disk
