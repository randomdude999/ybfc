// SPDX-License-Identifier: ISC
#ifndef _BFC_COMMON_H
#define _BFC_COMMON_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

void writebuf2(const byte* buf, size_t size);
#define writebuf(buf) writebuf2(buf, sizeof(buf))
void writebuf1(byte b);

static inline uint16_t le16i(uint16_t x) {
	union {
		uint16_t i;
		// this assumes CHAR_BIT=8.... but I don't think we're gonna get around
		// assuming that
		unsigned char b[2];
	} u;
	u.b[0] = x; u.b[1] = x>>8;
	return u.i;
}
static inline uint32_t le32i(uint32_t x) {
	union {
		uint32_t i;
		unsigned char b[4];
	} u;
	u.b[0] = x; u.b[1] = x>>8; u.b[2] = x>>16; u.b[3] = x>>24;
	return u.i;
}
static inline uint64_t le64i(uint64_t x) {
	union {
		uint64_t i;
		unsigned char b[8];
	} u;
	u.b[0] = x; u.b[1] = x>>8; u.b[2] = x>>16; u.b[3] = x>>24;
	u.b[4] = x>>32; u.b[5] = x>>40; u.b[6] = x>>48; u.b[7] = x>>56;
	return u.i;
}
#endif
