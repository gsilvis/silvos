#!/bin/bash
sed -r 's/(.*)(0x[0-9a-f]{16})(.*)/echo \1$(echo \2 | addr2line -f -e george.multiboot)\3/ge'
