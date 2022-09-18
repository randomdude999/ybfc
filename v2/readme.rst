a shitty brainfuck compiler
===========================

Only works on Linux (both the compiler and generated binaries).

Building: run `make`. requires gcc and nasm to be installed.

Usage: `./bfc <input file name> -o <output>`, where output defaults to a.out.
The tape size is also configurable with the `-t` option. However, due to
implementation details, it must be a power of 2, and can't be larger than 1GiB.
You can also change target architecture using `-m`, currently `i386` and `x64`
are implemented. Note that for x64, the tape size limit is 32TiB.

brainfuck dialect
=================

This implementation has a wrapping tape with a configurable length, defaulting
to 32KiB (close to the 30000 cells of the original compiler). Cell values are
8-bit with wrapping. The input routine leaves the cell unchanged on EOF.

implementation details (i386)
=============================

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

implementation details (x64)
============================

The general structure is similar to i386, along with the tape wrapping. However,
register allocation is different due to the different syscall interface.

rax - clobbered by input/output routines, also used for building 64-bit addresses
rbx - address of input subroutine (so each input command becomes `call rbx`)
rcx - address of trampoline routine for performing long jumps
rdx - always 1 (`dh=0` is used for comparison with 0, also used as buffer size by syscalls)
rsi - pointer to tape position
rdi - clobbered by input/output routines
rbp - address of output subroutine
r8 - address of another trampoline
r9 - pointer to start of tape buffer
r10 - pointer to end if tape minus 1

x86-64 has very few imm64 instructions, so most 64-bit literals need to go
through rax or similar.

Linux's x86-64 memory map means that the largest block that is suitable for the
tape is 32TiB, so this is the tape size limit. This would be a lot larger with
5-level page tables, but well, I don't have a CPU that supports them so I can't
test it.

TODO: document branch distance hacks
