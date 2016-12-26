#!/bin/bash

function join_by { local IFS="$1"; shift; echo "$*"; }

MODULES="$(join_by , george.multiboot "$@")"

qemu-system-x86_64 -kernel bootloader.multiboot -initrd "$MODULES" -hda temp_drive -S -s -serial stdio -device isa-debug-exit,iobase=0xf4,iosize=0x04
