v1/ contains a brainfuck compiler that outputs assembly files, and utilizes the
GNU assembler/linker. It targets 64-bit Linux, but is written in standard C.

v2/ contains a compiler which emits executables directly, and targets both
32-bit and 64-bit Linux. The generated output is a lot better than v1/, and it
can process arbitrarily large files in 1 pass.
