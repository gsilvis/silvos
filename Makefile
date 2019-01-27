KERNEL_OBJS := \
	$(patsubst kernel/%.c, kernel/%.o, $(wildcard kernel/*.c)) \
	$(patsubst kernel/%.s, kernel/%.o, $(wildcard kernel/*.s))

USERLAND_LIBS := \
	$(patsubst userland/lib/%.c, userland/lib/%.o, $(wildcard userland/lib/*.c)) \
	$(patsubst userland/lib/%.s, userland/lib/%.o, $(wildcard userland/lib/*.s))

TEST_PROGS := $(patsubst userland/%/expected.txt, %, $(wildcard userland/*/expected.txt))
UNIT_TESTS := $(patsubst tests/%/expected.txt, %, $(wildcard tests/*/expected.txt))

USERLAND_PROGS := $(patsubst userland/%/main.c, %, $(wildcard userland/*/main.c))

# Ideally, the kernel should compile and pass all tests with all of these
ifeq ($(origin KERNEL_OPT), undefined)
KERNEL_OPT := -O0
#KERNEL_OPT := -O1
#KERNEL_OPT := -O2
#KERNEL_OPT := -O3
#KERNEL_OPT := -Os
KERNEL_OPT += -g
#KERNEL_OPT += -flto
endif

ifeq ($(origin USER_OPT), undefined)
USER_OPT := -O0
USER_OPT += -g
endif

KERN_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -Wno-unused-const-variable -std=c99 -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel $(KERNEL_OPT)
KERN_LDFLAGS := -nostdlib -lgcc -Wl,-z,max-page-size=0x1000 -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2 $(KERNEL_OPT)

BOOTLOADER_CFLAGS := -ffreestanding -mno-red-zone -Wall -Wextra -std=c99 -O2 -g
BOOTLOADER_LDFLAGS := -nostdlib -Wl,--no-warn-mismatch -Wl,-z,max-page-size=0x1000

USER_CFLAGS := -ffreestanding -Wall -Wextra -Wno-main -std=c99 -Iuserland/include $(USER_OPT)
USER_LDFLAGS := -nostdlib -lgcc -Luserland/lib $(USER_OPT)

TEST_CFLAGS := -std=c99 -Wall -Wextra -DUNIT_TEST -Ikernel -g

TEST_CC := $(CC)
CC := x86_64-elf-gcc
AS := x86_64-elf-as
OBJCOPY := x86_64-elf-objcopy

# this instructs GNU make to use bash as our shell
SHELL = /bin/bash

# Primary targets

all: george.multiboot temp_drive \
	$(patsubst %, userland/%.bin, $(USERLAND_PROGS))

george.multiboot.elf64: $(KERNEL_OBJS)
	$(CC) -T kernel/kernel.ld $^ -o $@ $(KERN_LDFLAGS)

george.multiboot: george.multiboot.elf64
	$(OBJCOPY) $^ -F elf32-i386 $@


# Kernel Objects

kernel/%.o: kernel/%.c
	$(CC) -c $^ -o $@ $(KERN_CFLAGS)

kernel/%-asm.o: kernel/%-asm.s
	$(AS) $^ -o $@

# User Objects

userland/%/main.o: userland/%/main.c
	$(CC) -c $^ -o $@ $(USER_CFLAGS)

userland/%.bin: userland/%/main.o $(USERLAND_LIBS)
	$(CC) $^ -o $@ $(USER_LDFLAGS)

# Standalone tests, link against one kernel .c file.
tests/%/%.o: kernel/%.c
	$(TEST_CC) -c $^ -o $@ $(TEST_CFLAGS)

tests/%/main.o: tests/%/main.c
	$(TEST_CC) -c $^ -o $@ $(TEST_CFLAGS)

tests/%.bin: tests/%/main.o tests/%/%.o
	$(TEST_CC) $^ -o $@

## Special User Objects

userland/lib/%.o: userland/lib/%.c
	$(CC) -c $^ -o $@ $(USER_CFLAGS) -Iuserland/include

# Convenience

george.disk: george.multiboot menu.lst
	./script/MKDISK.sh

temp_drive:
	dd if=/dev/zero of=$@ bs=512 count=16 status=none

# Testing

test: user-tests unit-tests
user-tests: $(patsubst %, userland-test/%, $(TEST_PROGS))
unit-tests: $(patsubst %, tests/%, $(UNIT_TESTS))

.PRECIOUS: $(patsubst %, userland/%/output.txt, $(TEST_PROGS))
.PRECIOUS: $(patsubst %, tests/%/output.txt, $(UNIT_TESTS))

userland/%/test_disk:
	dd if=/dev/zero of=$@ bs=512 count=16 status=none

## This rule enforces that coverage.log requires output.txt to be built
## But doesn't do anything else
userland/%/coverage.log: userland/%/output.txt

# Runs without a GDB stub so make -j works properly.
userland/%/output.txt: userland/%.bin george.multiboot userland/%/test_disk
	./script/QEMU.sh \
		--no-gdb \
		--serial $@ \
		--no-graphic \
		--coverage $(subst output.txt,coverage.log,$@) \
		--drive $(subst output.txt,test_disk,$@) \
		$< || true

# FIXME: Don't sort before diffing expected and output.
# For now the diffing is sorting the inputs due to race
# conditions. This is a big hack, and prevents us from
# writing tests that SHOULD test for well-ordered outputs.
userland-test/%: userland/%/expected.txt userland/%/output.txt
	diff <(sort $(word 1,$^)) <(sort $(word 2,$^))
#	diff $^

# Non-userland tests won't have this problem
tests/%/output.txt: tests/%.bin
	./$^ > $@

tests/%: tests/%/expected.txt tests/%/output.txt
	diff $^

clean:
	rm -f \
		kernel/*.o \
		george.multiboot \
		george.multiboot.elf64 \
		george.disk \
		userland/*.bin \
		userland/*.o \
		userland/*/*.o \
		tests/*.bin \
		tests/*/*.o \
		temp_drive \
		userland/*/output.txt \
		userland/*/coverage.log \
		userland/*/test_disk \
		tests/*/output.txt \
