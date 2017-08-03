#!/bin/bash

OPTIONS=$(getopt -n "$0" -o hSks:d:c: --long "help,wait,serial:,nographic,drive:,coverage:,no-reboot,enable-kvm" -- "$@")

if [ $? -ne 0 ]; then
  echo "Failed to parse args"
  exit 1
fi

eval set -- "$OPTIONS"

function join_by { local IFS="$1"; shift; echo "$*"; }

PAUSE=0
KVM=0
NOREBOOT=0
SERIAL=stdio
NOGRAPHIC=0
DRIVE=temp_drive

while true; do
  case "$1" in
    -h|--help)
      echo "usage: $0 [opts] programs..."
      echo ""
      echo "  -h, --help          Print this help message"
      echo "  -d, --drive=FILE    Drive to use (default=temp_drive)"
      echo "  -S, --wait          Pause CPU on startup"
      echo "  -s, --serial=FILE   Output serial to FILE instead of stdout"
      echo "  -c, --coverage=FILE Output unique hit kernel mode %rip values to FILE"
      echo "      --nographic     Don't display the graphics (default is SDL or curses)"
      exit 1
      shift;;
    -S|--wait)
      PAUSE=1
      shift;;
    -k|--enable-kvm)
      KVM=1
      shift;;
    --no-reboot)
      NOREBOOT=1
      shift;;
    -s|--serial)
      SERIAL="file:$2"
      shift 2;;
    --nographic)
      NOGRAPHIC=1
      shift;;
    -d|--drive)
      DRIVE="$2"
      shift 2;;
    -c|--coverage)
      COVERAGE="$2"
      shift 2;;
    --) shift; break;;
    *) break ;;
  esac
done

MODULES="$(join_by , george.multiboot "$@")"

QEMU_ARGS=" -kernel bootloader.multiboot"
QEMU_ARGS+=" -initrd $MODULES"
QEMU_ARGS+=" -m 128"
QEMU_ARGS+=" -drive file=$DRIVE,index=0,media=disk,format=raw"
QEMU_ARGS+=" -serial $SERIAL"
QEMU_ARGS+=" -device isa-debug-exit,iobase=0xf4,iosize=0x04"
# Expose standard gdb on :1234
QEMU_ARGS+=" -s"
# Pause on startup (require 'c' on console, or 'c' in GDB to start)
if [ "$PAUSE" -gt 0 ]; then
  QEMU_ARGS+=" -S"
fi

if [ "$NOGRAPHIC" -gt 0 ]; then
  QEMU_ARGS+=" -nographic -display none"
# If DISPLAY isn't set, use -curses
elif [ -z "$DISPLAY" ]; then
  QEMU_ARGS+=" -curses"
fi

if [ "$NOREBOOT" -gt 0 ]; then
  QEMU_ARGS+=" -no-reboot"
fi

if [ "$KVM" -gt 0 ]; then
  QEMU_ARGS+=" --enable-kvm"
fi

#echo "$QEMU_ARGS" >&1

if [ -z "$COVERAGE" ]; then
  qemu-system-x86_64 $QEMU_ARGS
  exit $?
else
  qemu-system-x86_64 $QEMU_ARGS -d in_asm -D >( grep -E "^(Trace.*\[ffffffff|IN:|0xffffff)" | uniq > "$COVERAGE")
  exit $?
fi
