#include "bfc_common.h"

#include "header_x64.h"

static void reloc64(size_t reloc_at, uint64_t data) {
	size_t old_out_pos = out_off;
	out_off = reloc_at;
	fseek(output, out_off, SEEK_SET);
	writebuf(B(LE64(data)));
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}

void write_x64_header(size_t tape_size) {
	writebuf(header_bin);
	reloc64(header_tape_size, tape_size);
	reloc64(header_tape_andmask, header_tape_addr + tape_size - 1);
}

void write_x64_end() {
            // xor eax,eax; mov al, 60; xor edi, edi; syscall
	writebuf(B(0x31, 0xc0, 0xb0, 0x3c, 0x31, 0xff, 0x0f, 0x05));
	uint64_t fsz = out_off;
	// twice because this is both the memory and file size
	reloc64(header_file_size_loc, fsz);
	reloc64(header_file_size_loc+8, fsz);
}

#define RLE_BYTE(bytes1, bytesn) if(run_length == 1) writebuf(bytes1); \
	else writebuf(bytesn);
#define RLE_LONG(bytes1, bytes2, bytes3, byteslong) \
	if(run_length == 1) writebuf(bytes1); \
	else if(run_length < 128) writebuf(bytes2); \
	else if(run_length < 1ull<<31) writebuf(bytes3); \
	else writebuf(byteslong);

void write_x64_cmd_inc_run(uint8_t run_length) {
	//     inc byte [rsi]       add byte [rsi], imm8
	RLE_BYTE(B(0xfe, 0x06), B(0x80, 0x06, run_length));
}

void write_x64_cmd_dec_run(uint8_t run_length) {
	//     dec byte [rsi]       sub byte [rsi], imm8
	RLE_BYTE(B(0xfe, 0x0e), B(0x80, 0x2e, run_length));
}

void write_x64_cmd_l_run(size_t run_length) {
	// dec rsi; sub rsi, imm8; sub rsi, imm32
	// last one is mov rax, imm64; sub rsi, rax
	RLE_LONG(B(0x48, 0xff, 0xce), B(0x48, 0x83, 0xee, run_length),
	         B(0x48, 0x81, 0xee, LE32(run_length)),
	         B(0x48, 0xb8, LE64(run_length), 0x48, 0x29, 0xc6));
	// and rsi, r9; or rsi, r10
	writebuf(B(0x4c, 0x21, 0xce, 0x4c, 0x09, 0xd6));
}

void write_x64_cmd_r_run(size_t run_length) {
	// inc rsi; add rsi, imm8; add rsi, imm32
	// last one is mov rax, imm64; add rsi, rax
	RLE_LONG(B(0x48, 0xff, 0xc6), B(0x48, 0x83, 0xc6, run_length),
	         B(0x48, 0x81, 0xc6, LE32(run_length)),
	         B(0x48, 0xb8, LE64(run_length), 0x48, 0x01, 0xc6));
	// and rsi, r9
	writebuf(B(0x4c, 0x21, 0xce));
}

void write_x64_cmd_inp() {
	writebuf(B(0xff, 0xd3)); // call rbx
}

void write_x64_cmd_out() {
	writebuf(B(0xff, 0xd5)); // call rbp
}

                       // cmp bh, [rsi]; je <relocated addr>
static byte start_loop_asm[] = { 0x3a, 0x3e, 0x0f, 0x84, 42, 42, 42, 42 };

void write_x64_start_loop() {
	writebuf(start_loop_asm);
}

void write_x64_end_loop(size_t loop_tgt) {
	size_t reloc_at = loop_tgt + sizeof(start_loop_asm) - 4;
	ssize_t distance = loop_tgt - (out_off + 5);
	if(-distance > 1ll << 31)
		error("error: what is wrong with you");
	writebuf(B(0xe9, LE32(distance)));
	size_t old_out_pos = out_off;
	fseek(output, reloc_at, SEEK_SET);
	writebuf(B(LE32(out_off - (reloc_at+4))));
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}
