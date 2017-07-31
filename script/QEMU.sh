#!/bin/bash

OPTIONS=$(getopt -n "$0" -o hS: --long "help,wait:" -- "$@")

if [ $? -ne 0 ]; then
  echo "Failed to parse args"
  exit 1
fi

eval set -- "$OPTIONS"

function join_by { local IFS="$1"; shift; echo "$*"; }

PAUSE=0

while true; do
  case "$1" in
    -h|--help)
      echo "usage: $0 [opts] programs..."
      echo ""
      echo "  -h --help       Print this help message"
      echo "  -S --wait       Pause CPU on startup"
      exit 1
      shift;;
    -S|--wait)
      PAUSE=1
      shift;;
    --)
      shift
      break;;
  esac
done

MODULES="$(join_by , george.multiboot "$@")"

QEMU_ARGS=" -kernel bootloader.multiboot -initrd $MODULES"
QEMU_ARGS+=" -drive file=temp_drive,index=0,media=disk,format=raw"
QEMU_ARGS+=" -serial stdio"
QEMU_ARGS+=" -device isa-debug-exit,iobase=0xf4,iosize=0x04"
# Expose standard gdb on :1234
QEMU_ARGS+=" -s"
# Pause on startup (require 'c' on console, or 'c' in GDB to start)
if [ "$PAUSE" -gt 0 ]; then
  QEMU_ARGS+=" -S"
fi

# If DISPLAY isn't set, use -curses
if [ -z "$DISPLAY" ]; then
  QEMU_ARGS+=" -curses"
fi

exec qemu-system-x86_64 $QEMU_ARGS
