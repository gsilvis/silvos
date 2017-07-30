#!/bin/bash

set -eu

for oflag in -O0 -O2 -O3 -Os; do
  for ltoflag in -flto ''; do
    for uoflag in -O0 -O2; do
      make clean
      USER_OPT="$uoflag" KERNEL_OPT="$oflag -g $ltoflag " make -j $(nproc) test
     done
  done
done
