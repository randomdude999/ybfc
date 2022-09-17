#include "arch_i386.h"
#include "bfc_common.h"

#include "header.h"


static void writejmpto(byte opc, uint32_t target) {
	uint32_t offset = target - (out_off+5);
	byte buf[5] = {opc, offset, offset >> 8, offset >> 16, offset >> 24};
	writebuf(buf);
}

void write_i386_header(size_t tape_size) {
	writebuf(header_bin);
	reloc32(header_tape_size, tape_size);
	reloc32(header_tape_andmask, header_tape_addr + tape_size - 1);
}

void write_i386_end() {
            // xor eax,eax; inc eax; xor ebx,ebx; int 0x80
	writebuf(B(0x31, 0xc0, 0x40, 0x31, 0xdb, 0xcd, 0x80));
	uint32_t fsz = out_off;
	// twice because this is both the memory and file size
	reloc32(header_file_size_loc, fsz);
	reloc32(header_file_size_loc+4, fsz);
}

#define RLE_BYTE(bytes1, bytesn) if(run_length == 1) writebuf(bytes1); \
	else writebuf(bytesn);
#define RLE_LONG(bytes1, bytes2, byteslong) \
	if(run_length == 1) writebuf(bytes1); \
	else if(run_length < 128) writebuf(bytes2); \
	else writebuf(byteslong);

void write_i386_cmd_inc_run(uint8_t run_length) {
	//     inc byte [ecx]       add byte [ecx], imm8
	RLE_BYTE(B(0xfe, 0x01), B(0x80, 0x01, run_length));
}

void write_i386_cmd_dec_run(uint8_t run_length) {
	//     dec byte [ecx]       sub byte [ecx], imm8
	RLE_BYTE(B(0xfe, 0x09), B(0x80, 0x29, run_length));
}

void write_i386_cmd_l_run(size_t run_length) {
	//       dec ecx    sub ecx, imm8              sub ecx, imm32
	RLE_LONG(B(0x49), B(0x83, 0xe9, run_length), B(0x81, 0xe9, ENC_LE(run_length)));
	// and ecx, edi; or ecx, esi
	writebuf(B(0x21, 0xf9, 0x09, 0xf1));
}

void write_i386_cmd_r_run(size_t run_length) {
	//       inc ecx    add ecx, imm8              add ecx, imm32
	RLE_LONG(B(0x41), B(0x83, 0xc1, run_length), B(0x81, 0xc1, ENC_LE(run_length)));
	// and ecx, edi
	writebuf(B(0x21, 0xf9));
}

void write_i386_cmd_inp() {
	writejmpto(0xe8, header_sub_input);
}

void write_i386_cmd_out() {
	writejmpto(0xe8, header_sub_output);
}

                       // cmp bh, [ecx]; je <relocated addr>
byte start_loop_asm[] = { 0x3a, 0x39, 0x0f, 0x84, 42, 42, 42, 42 };

void write_i386_start_loop() {
	writebuf(start_loop_asm);
}

void write_i386_end_loop(size_t loop_tgt) {
	uint32_t reloc_at = loop_tgt + sizeof(start_loop_asm) - 4;
	writejmpto(0xe9, loop_tgt);
	reloc32(reloc_at, out_off - (reloc_at+4));
}
