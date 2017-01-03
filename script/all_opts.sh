#!/bin/bash

set -eu

for oflag in -O0 -O1 -O2 -O3 -Os; do
  for ltoflag in -flto ''; do
    KERNEL_OPT="$oflag -g $ltoflag " make clean test
  done
done
