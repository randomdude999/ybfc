// SPDX-License-Identifier: ISC
#include "arch_common.h"
#include "bfc_common.h"
//#include "buf_writer.h"
#include "header_win32.h"
#include <stdint.h>

static void reloc32(size_t reloc_at, uint32_t data) {
	uint32_t old_out_pos = out_off;
	out_off = reloc_at;
	fseek(output, out_off, SEEK_SET);
	writebuf(B(LE32(data)));
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}

void arch_win32_write_header(size_t tape_size) {
	writebuf(header_bin);
	reloc32(header_tape_size, (uint32_t)tape_size);
}

void arch_win32_end() {
			// xor eax,eax; ret
	writebuf(B(0x31, 0xc0, 0xc3));
	uint32_t code_size = out_off - header_code_section_start;
	uint32_t padding = 0x200 - (out_off & 0x1ff);
	if(padding == 0x200) padding = 0;
	for(int i=0; i<padding; i++) writebuf1(0); // ugh
	uint32_t code_phys_size = out_off - header_code_section_start;
	reloc32(header_code_size_virt, code_size);
	reloc32(header_code_size_phys_1, code_phys_size);
	reloc32(header_code_size_phys_2, code_phys_size);
	uint32_t code_size_sections = (code_size + 0xfff) & ~0xfff;
	reloc32(header_size_of_image, 0x2000 + code_size_sections);
}

#define RLE_BYTE(bytes1, bytesn) if(run_length == 1) writebuf(bytes1); \
	else writebuf(bytesn);
#define RLE_LONG(bytes1, bytes2, byteslong) \
	if(run_length == 1) writebuf(bytes1); \
	else if(run_length < 128) writebuf(bytes2); \
	else writebuf(byteslong);

void arch_win32_cmd_inc_run(uint8_t run_length) {
	//     inc byte [eax]       add byte [eax], imm8
	RLE_BYTE(B(0xfe, 0x00), B(0x80, 0x00, run_length));
}

void arch_win32_cmd_dec_run(uint8_t run_length) {
	//     dec byte [eax]       sub byte [eax], imm8
	RLE_BYTE(B(0xfe, 0x08), B(0x80, 0x28, run_length));
}

void arch_win32_cmd_l_run(size_t run_length) {
	//       dec eax    sub eax, imm8              sub eax, imm32
	RLE_LONG(B(0x48), B(0x83, 0xe8, run_length), B(0x2d, LE32(run_length)));
	// cmp eax, ebx; jae .skip; add eax, ebp; .skip:
	writebuf(B(0x39, 0xd8, 0x73, 0x02, 0x01, 0xe8));
}

void arch_win32_cmd_r_run(size_t run_length) {
	//       inc eax    add eax, imm8              add eax, imm32
	RLE_LONG(B(0x40), B(0x83, 0xc0, run_length), B(0x05, LE32(run_length)));
	// cmp eax, ecx; jb .skip; sub eax, ebp; .skip:
	writebuf(B(0x39, 0xc8, 0x72, 0x02, 0x29, 0xe8));
}

void arch_win32_cmd_inp() {
	writebuf(B(0xe8, LE32(header_input_loc - out_off - 5)));
}

void arch_win32_cmd_out() {
	writebuf(B(0xe8, LE32(header_output_loc - out_off - 5)));
}

                       // cmp [eax], dl; je <relocated addr>
static byte start_loop_asm[] = { 0x38, 0x10, 0x0f, 0x84, 42, 42, 42, 42 };

void arch_win32_start_loop() {
	writebuf(start_loop_asm);
}

void arch_win32_end_loop(size_t loop_tgt) {
	uint32_t reloc_at = loop_tgt + sizeof(start_loop_asm) - 4;
	// jmp <loop start address>
	writebuf(B(0xe9, LE32(loop_tgt - out_off - 5)));
	reloc32(reloc_at, out_off - (reloc_at+4));
}

void arch_win32_post_cmd() {
	// i have no clue how large a code section you can actually allocate
	/*if(out_off >= TAPE_ADDR - CODE_START)
		error("error: code section too large");*/
}

void arch_win32_get_tape_max(size_t *out) {
	// the real limit is probably lower, but the kernel will yell at us if we
	// try to allocate too much
	*out = UINT32_MAX;
}
