To build, you must have a minimal cross-compiler for x86_64-elf. Follow the 
directions at http://wiki.osdev.org/GCC_Cross-Compiler for target x86_64-elf to set 
one up, and make sure it is in your path. When you are set up `make` should be 
sufficient.

To run, execute:

    ./script/TEST-QEMU.sh & ./script/TEST-GDB.sh
