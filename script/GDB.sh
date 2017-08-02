#!/bin/sh

gdb \
  -ex "set confirm off" \
  -ex "file bootloader.multiboot" \
  -ex "target remote localhost:1234" \
  -ex "break _start64" \
  -ex "c" \
  -ex "disconnect" \
  -ex "set arch i386:x86-64" \
  -ex "file george.multiboot" \
  -ex "target remote localhost:1234" \
  -ex "break kernel_main" \
  -ex "c" \
  -ex "del break 1" \
  -ex "set confirm on"
