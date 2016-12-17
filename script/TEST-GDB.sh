gdb \
  -ex "file george.multiboot" \
  -ex "target remote localhost:1234" \
  -ex "break kernel_main" \
  -ex "c" \
  -ex "disconnect" \
  -ex "set arch i386:x86-64" \
  -ex "target remote localhost:1234" \
  -ex "del break 1"
