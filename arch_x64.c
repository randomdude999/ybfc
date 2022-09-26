#include "bfc_common.h"

#define ELF_BITS 64
#include "elf_writer.c"

static void reloc64(size_t reloc_at, uint64_t data) {
	size_t old_out_pos = out_off;
	out_off = reloc_at;
	fseek(output, out_off, SEEK_SET);
	writebuf(B(LE64(data)));
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}

#define TAPE_ADDR 0x200000000000ull


void* header_write_locs_x64;
void arch_x64_write_header(size_t tape_size) {
	uint64_t TAPE_ANDMASK = TAPE_ADDR + tape_size - 1;
#include "header_x64.h"
	header_write_locs_x64 = write_elf_hdr(0x01000000, TAPE_ADDR, tape_size, 1, sizeof(prelude_lib), 62);
	writebuf(prelude_lib);
	writebuf(prelude_start);
}

void arch_x64_end() {
            // xor eax,eax; mov al, 60; xor edi, edi; syscall
	writebuf(B(0x31, 0xc0, 0xb0, 0x3c, 0x31, 0xff, 0x0f, 0x05));
	finalize_elf_header(header_write_locs_x64);
}

#define RLE_BYTE(bytes1, bytesn) if(run_length == 1) writebuf(bytes1); \
	else writebuf(bytesn);
#define RLE_LONG(bytes1, bytes2, bytes3, byteslong) \
	if(run_length == 1) writebuf(bytes1); \
	else if(run_length < 128) writebuf(bytes2); \
	else if(run_length < 1ull<<31) writebuf(bytes3); \
	else writebuf(byteslong);

void arch_x64_cmd_inc_run(uint8_t run_length) {
	//     inc byte [rsi]       add byte [rsi], imm8
	RLE_BYTE(B(0xfe, 0x06), B(0x80, 0x06, run_length));
}

void arch_x64_cmd_dec_run(uint8_t run_length) {
	//     dec byte [rsi]       sub byte [rsi], imm8
	RLE_BYTE(B(0xfe, 0x0e), B(0x80, 0x2e, run_length));
}

void arch_x64_cmd_l_run(size_t run_length) {
	// dec rsi; sub rsi, imm8; sub rsi, imm32
	// last one is mov rax, imm64; sub rsi, rax
	RLE_LONG(B(0x48, 0xff, 0xce), B(0x48, 0x83, 0xee, run_length),
	         B(0x48, 0x81, 0xee, LE32(run_length)),
	         B(0x48, 0xb8, LE64(run_length), 0x48, 0x29, 0xc6));
	// and rsi, r9; or rsi, r10
	writebuf(B(0x4c, 0x21, 0xce, 0x4c, 0x09, 0xd6));
}

void arch_x64_cmd_r_run(size_t run_length) {
	// inc rsi; add rsi, imm8; add rsi, imm32
	// last one is mov rax, imm64; add rsi, rax
	RLE_LONG(B(0x48, 0xff, 0xc6), B(0x48, 0x83, 0xc6, run_length),
	         B(0x48, 0x81, 0xc6, LE32(run_length)),
	         B(0x48, 0xb8, LE64(run_length), 0x48, 0x01, 0xc6));
	// and rsi, r9
	writebuf(B(0x4c, 0x21, 0xce));
}

void arch_x64_cmd_inp() {
	writebuf(B(0xff, 0xd3)); // call rbx
}

void arch_x64_cmd_out() {
	writebuf(B(0xff, 0xd5)); // call rbp
}

void arch_x64_start_loop() {
	// cmp dh, [rsi]; je <relocated addr>
	// note: it's important that this is 8 bytes for the long jumps to work
	writebuf(B(0x3a, 0x36, 0x0f, 0x84, 42, 42, 42, 42));
}

#define LE48(x) LE32(x), (byte)(x>>32), (byte)(x>>40)

void arch_x64_end_loop(size_t loop_tgt) {
	ssize_t distance = loop_tgt - (out_off + 5);
	int do_long_jump = (-distance > 1ll << 31);
	if(do_long_jump) {
		// need to do a long jump
		// note that this condition is on the backwards jump not fitting, but
		// the backwards jump is always longer, so if the forwards jump
		// overflowed then the backwards one does too
		uint64_t back_jmp_offset = loop_tgt - (out_off + 3);
		// call r8; <6-byte offset>
		writebuf(B(0x41, 0xff, 0xd0, LE48(back_jmp_offset)));
	} else {
		// jmp <offset>
		writebuf(B(0xe9, LE32(distance)));
	}
	
	size_t old_out_pos = out_off;
	if(do_long_jump) {
		uint64_t fwd_jmp_offset = out_off - (loop_tgt + 2);
		fseek(output, loop_tgt, SEEK_SET);
		// call rcx; <6-byte offset>
		writebuf(B(0xff, 0xd1, LE48(fwd_jmp_offset)));
	} else {
		size_t reloc_at = loop_tgt + 4;
		fseek(output, reloc_at, SEEK_SET);
		writebuf(B(LE32(out_off - (reloc_at+4))));
	}
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}

void arch_x64_post_cmd() {
	if(out_off >= TAPE_ADDR - 0x01000000)
		error("error: code section too large");
}

void arch_x64_get_tape_max(size_t *out) {
	*out = (TAPE_ADDR & -TAPE_ADDR) >> 1;
}
