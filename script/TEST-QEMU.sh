#!/bin/bash

function join_by { local IFS="$1"; shift; echo "$*"; }

MODULES="$(join_by , george.multiboot "$@")"

qemu-system-x86_64 -kernel bootloader.multiboot -initrd "$MODULES" -drive file=temp_drive,index=0,media=disk,format=raw -S -s -serial stdio -device isa-debug-exit,iobase=0xf4,iosize=0x04
