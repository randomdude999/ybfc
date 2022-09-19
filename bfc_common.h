#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define error(x, ...) (fprintf(stderr, x "\n", ## __VA_ARGS__), exit(1))
#define byte uint8_t
#define B(...) ((byte[]){ __VA_ARGS__ })
#define LE32(val) (byte)(val), (byte)((val) >> 8), (byte)((val) >> 16), (byte)((val) >> 24)
#define LE64(val) LE32(val), LE32((val) >> 32)

extern FILE* output;
extern size_t out_off;
extern int current_arch;

#define ARCH_i386 0
#define ARCH_x64 1

void writebuf2(byte* buf, size_t size);
#define writebuf(buf) writebuf2(buf, sizeof(buf))
void writebuf1(byte b);
