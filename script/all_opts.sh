#!/bin/bash

set -eu

for oflag in '-O0' '-O2' '-O3' '-O3 -flto'; do
  for uoflag in -O0 -O2; do
    make clean
    USER_OPT="$uoflag" KERNEL_OPT="$oflag -g" make -j $(nproc) test
   done
done
