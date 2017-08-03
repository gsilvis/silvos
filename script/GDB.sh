#!/bin/sh

gdb \
  -ex "set confirm off" \
  -ex "file bootloader.multiboot" \
  -ex "target remote localhost:1234" \
  -ex "hbreak _start64" \
  -ex "c" \
  -ex "del break 1" \
  -ex "disconnect" \
  -ex "set arch i386:x86-64" \
  -ex "file george.multiboot" \
  -ex "target remote localhost:1234" \
  -ex "hbreak kernel_main" \
  -ex "c" \
  -ex "del break 2" \
  -ex "set confirm on"
