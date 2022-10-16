ybfc
====

Short for "y brainfuck compiler", because the entire project should make you ask
"but y tho".

The compiler should work on all Unices, the generated executables are currently
only 32-bit or 64-bit Linux.

Building: run ``make``. requires gcc and nasm to be installed.

Usage: ``./ybfc <input file name> -o <output>``, where output defaults to a.out.
The tape size is also configurable with the ``-t`` option. However, due to
implementation details, it must be a power of 2, and can't be larger than 1GiB.
You can also change target architecture using ``-m``, currently ``i386`` and
``x64`` are implemented. Note that for x64, the tape size limit is 16TiB.

brainfuck dialect
-----------------

This implementation has a wrapping tape with a configurable length, defaulting
to 32KiB (close to the 30000 cells of the original compiler). Cell values are
8-bit with wrapping. The input routine leaves the cell unchanged on EOF.

implementation details (i386)
-----------------------------

All registers have a fixed purpose during the entire execution of the program:

| eax - reserved for the input/output routines
| ebx - reserved for the input/output routines
| ecx - pointer to the current position on the tape
| edx - always 1 (used as the buffer size by the input/output syscalls)
| esi - pointer to the start of the tape buffer
| edi - pointer to the end of the tape buffer minus 1

Tape wrapping is implemented with bitwise operations: the tape size is a power
of 2, and the implementation puts the tape at an address with all low bits
cleared. This means that wrapping on the right edge can be done by a single
``and`` instruction, to mask out the bit which became a 1 due to the overflow.
Left edge wrapping on the other hand needs both an ``and`` and an ``or``, to
reset the high bits correctly.

The registers are laid out such that syscalls can be performed without moving
any data between registers. ``eax`` and ``ebx`` are overwritten by the
input/output routines with the syscall number and file descriptor.

Note that due to the layout of the memory map, the tape size is limited to 1GiB,
and compiled program size to 2GiB.

implementation details (x64)
----------------------------

The general structure is similar to i386, along with the tape wrapping. However,
register allocation is different due to the different syscall interface.

| rax - clobbered by input/output routines, also used for building 64-bit addresses
| rbx - address of input subroutine (so each input command becomes ``call rbx``)
| rcx - address of trampoline routine for performing long jumps
| rdx - always 1 (``dh=0`` is used for testing 0, also used as buffer size by syscalls)
| rsi - pointer to tape position
| rdi - clobbered by input/output routines
| rbp - address of output subroutine
| r8 - address of another trampoline
| r9 - pointer to start of tape buffer
| r10 - pointer to end if tape minus 1

x86-64 has very few imm64 instructions, so most 64-bit literals need to go
through rax or similar.

Linux's x86-64 memory map means that the largest block that is suitable for the
tape is 16TiB, so this is the tape size limit. This would be a lot larger with
5-level page tables, but well, I don't have a CPU that supports them so I can't
test it. Also note that due to the memory map, the maximum size of the emitted
executable code is around 32TiB.

x86-64 makes branches to more than 2GB away a bit difficult. Nevertheless, the
compiler can emit any possible loop, as long as the full program fits within
the memory map. This comes at no cost to short loops. Short loops have an
8-byte beginning, which tests the current tape value and branches if it's zero.
Long loops replace this with a 2-byte ``call rcx`` instruction followed by a
6-byte offset to the end of the loop. This means the compiler can allocate 8
bytes for the start of any loop without knowing how long it is. ``rcx``
contains the address of a trampoline routine that uses the return pointer to
read said 6-byte offset and jump to either into or past the loop. The end of a
long loop is similar, but it uses ``call r8``, which jumps to the middle of the
trampoline, skipping the condition check.

implementation details (win32)
------------------------------

The win32 implementation works mostly like i386, except the tape wrapping is
implemented in a more basic way and doesn't require tape size to be a power of
2 (though this means the emitted code is a little longer). The register
allocation is as follows:

| eax - current tape offset
| ebx - start of tape
| ecx - end of tape (one byte past)
| edx - always 0
| esi - stdin handle
| edi - stdout handle
| ebp - tape length

In order to conform with brainfuck specifications, input and output should
always be done with Unix-style line endings. This is implemented by simply
skipping all ``\r``-s in the input. As for output, if the standard output stream
is opened in text mode then Windows automatically converts ``\n`` to ``\r\n``.
