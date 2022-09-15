a shitty brainfuck compiler
===========================

Only works on Linux (both the compiler and generated binaries). Also note that
it outputs 32-bit binaries. This shouldn't cause a problem though, as they are
statically linked, so you don't need to install any 32-bit libraries to run the
outputs.

Building: run `make`. requires gcc and nasm to be installed.

Usage: `./bfc <input file name> -o <output>`, where output defaults to a.out.
The tape size is also configurable with the `-t` option. However, due to
implementation details, it must be a power of 2, and can't be larger than 1GiB.
This shouldn't be a problem though, as most brainfuck interpreters don't give
you nearly as much space.

brainfuck dialect
=================

This implementation has a wrapping tape with a configurable length, defaulting
to 32KiB (close to the 30000 cells of the original compiler). Cell values are
8-bit with wrapping. The input routine leaves the cell unchanged on EOF.

implementation details
======================

All registers have a fixed purpose during the entire execution of the program:

eax - reserved for the input/output routines
ebx - reserved for the input/output routines
ecx - pointer to the current position on the tape
edx - always 1 (used as the buffer size by the input/output syscalls)
esi - pointer to the start of the tape buffer
edi - pointer to the end of the tape buffer minus 1

Tape wrapping is implemented with bitwise operations: the tape size is a power
of 2, and the implementation puts the tape at an address with all low bits
cleared. This means that wrapping on the right edge can be done by a single
`and` instruction, to mask out the bit which became a 1 due to the overflow.
Left edge wrapping on the other hand needs both an `and` and an `or`, to reset
the high bits correctly.

The registers are laid out such that syscalls can be performed without moving
any data between registers. `eax` and `ebx` are overwritten by the input/output
routines with the syscall number and file descriptor.
