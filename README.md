To build, you must have a minimal cross-compiler for x86_64-elf. Follow the 
directions at http://wiki.osdev.org/GCC_Cross-Compiler for target x86_64-elf to set 
one up, and make sure it is in your path. When you are set up `make` should be 
sufficient.

To run, execute:

    ./script/QEMU.sh userland/calc.bin

Or whatever other userland program(s) you would like.

# Contributors

- [George Silvis III](https://github.com/gsilvis)
- [Colin Stanfill](https://github.com/cstanfill)
- [Allan Wirth](https://github.com/allanlw)
