// SPDX-License-Identifier: ISC
#include "arch_common.h"
#include "bfc_common.h"

#define ELF_BITS 32
#include "elf_writer.c"

static void reloc32(size_t reloc_at, uint32_t data) {
	uint32_t old_out_pos = out_off;
	out_off = reloc_at;
	fseek(output, out_off, SEEK_SET);
	byte buf[4] = { data, data >> 8, data >> 16, data >> 24 };
	writebuf(buf);
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}

#define TAPE_ADDR 0x80000000
#define CODE_START 0x01000000

static void* header_write_locs;
uint32_t input_loc, output_loc;
void arch_i386_write_header(size_t tape_size) {
	uint32_t TAPE_ANDMASK = TAPE_ADDR + tape_size - 1;
#include "header_i386.h"
	header_write_locs = write_elf_hdr(CODE_START, TAPE_ADDR, tape_size, 1, sizeof(prelude_func_input)+sizeof(prelude_func_output), 3);
	input_loc = out_off;
	writebuf(prelude_func_input);
	output_loc = out_off;
	writebuf(prelude_func_output);
	writebuf(prelude_start);
}

void arch_i386_end() {
            // xor eax,eax; inc eax; xor ebx,ebx; int 0x80
	writebuf(B(0x31, 0xc0, 0x40, 0x31, 0xdb, 0xcd, 0x80));
	finalize_elf_header(header_write_locs);
}

#define RLE_BYTE(bytes1, bytesn) if(run_length == 1) writebuf(bytes1); \
	else writebuf(bytesn);
#define RLE_LONG(bytes1, bytes2, byteslong) \
	if(run_length == 1) writebuf(bytes1); \
	else if(run_length < 128) writebuf(bytes2); \
	else writebuf(byteslong);

void arch_i386_cmd_inc_run(uint8_t run_length) {
	//     inc byte [ecx]       add byte [ecx], imm8
	RLE_BYTE(B(0xfe, 0x01), B(0x80, 0x01, run_length));
}

void arch_i386_cmd_dec_run(uint8_t run_length) {
	//     dec byte [ecx]       sub byte [ecx], imm8
	RLE_BYTE(B(0xfe, 0x09), B(0x80, 0x29, run_length));
}

void arch_i386_cmd_l_run(size_t run_length) {
	//       dec ecx    sub ecx, imm8              sub ecx, imm32
	RLE_LONG(B(0x49), B(0x83, 0xe9, run_length), B(0x81, 0xe9, LE32(run_length)));
	// and ecx, edi; or ecx, esi
	writebuf(B(0x21, 0xf9, 0x09, 0xf1));
}

void arch_i386_cmd_r_run(size_t run_length) {
	//       inc ecx    add ecx, imm8              add ecx, imm32
	RLE_LONG(B(0x41), B(0x83, 0xc1, run_length), B(0x81, 0xc1, LE32(run_length)));
	// and ecx, edi
	writebuf(B(0x21, 0xf9));
}

void arch_i386_cmd_inp() {
	writebuf(B(0xe8, LE32(input_loc - out_off - 5)));
}

void arch_i386_cmd_out() {
	writebuf(B(0xe8, LE32(output_loc - out_off - 5)));
}

                       // cmp bh, [ecx]; je <relocated addr>
static byte start_loop_asm[] = { 0x3a, 0x39, 0x0f, 0x84, 42, 42, 42, 42 };

void arch_i386_start_loop() {
	writebuf(start_loop_asm);
}

void arch_i386_end_loop(size_t loop_tgt) {
	uint32_t reloc_at = loop_tgt + sizeof(start_loop_asm) - 4;
	writebuf(B(0xe9, LE32(loop_tgt - out_off - 5)));
	reloc32(reloc_at, out_off - (reloc_at+4));
}

void arch_i386_post_cmd() {
	if(out_off >= TAPE_ADDR - CODE_START)
		error("error: code section too large");
}

void arch_i386_get_tape_max(size_t *out) {
	*out = (TAPE_ADDR & -TAPE_ADDR) >> 1;
}
