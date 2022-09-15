#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

#include "header.h"

#define error(x, ...) (fprintf(stderr, x "\n", ## __VA_ARGS__), exit(1))

#define byte uint8_t

byte inc_asm[] = { 0xfe, 0x01 }; // inc byte [ecx]
byte dec_asm[] = { 0xfe, 0x09 }; // dec byte [ecx]
                  // and ecx, edi; or ecx, esi
byte left_wrap_asm[]  = { 0x21, 0xf9, 0x09, 0xf1 };
                  // and ecx, edi
byte right_wrap_asm[] = { 0x21, 0xf9 };
                       // cmp bh, [ecx]; je <relocated addr>
byte start_loop_asm[] = { 0x3a, 0x39, 0x0f, 0x84, 42, 42, 42, 42 };
                // xor eax,eax; inc eax; xor ebx,ebx; int 0x80
byte fin_asm[] = { 0x31, 0xc0, 0x40, 0x31, 0xdb, 0xcd, 0x80};

uint32_t loopstack[256];
int loopdepth = 0;
int run_length = 0;
char run_type;
FILE* output;
uint32_t out_off;

void writebuf2(byte* buf, size_t size) {
	if(fwrite(buf, 1, size, output) != size)
		error("error writing to output: %s", strerror(errno));
	out_off += size;
}
#define writebuf(buf) writebuf2(buf, sizeof(buf))
void writebuf1(byte b) {
	byte arr[1] = {b};
	writebuf2(arr, 1);
}
void writejmpto(byte opc, uint32_t target) {
	uint32_t offset = target - (out_off+5);
	byte buf[5] = {opc, offset, offset >> 8, offset >> 16, offset >> 24};
	writebuf(buf);
}

void reloc32(uint32_t reloc_at, uint32_t data) {
	uint32_t old_out_pos = out_off;
	out_off = reloc_at;
	fseek(output, out_off, SEEK_SET);
	byte buf[4] = { data, data >> 8, data >> 16, data >> 24 };
	writebuf(buf);
	out_off = old_out_pos;
	fseek(output, out_off, SEEK_SET);
}

void end_run() {
	if(run_length == 0) return;
	if(run_type == '+') {
		if(run_length == 1) writebuf(inc_asm);
		else {
			// add byte [ecx], imm8
			byte buf[] = { 0x80, 0x01, run_length };
			writebuf(buf);
		}
	}
	else if(run_type == '-') {
		if(run_length == 1) writebuf(dec_asm);
		else {
			// sub byte [ecx], imm8
			byte buf[] = { 0x80, 0x29, run_length };
			writebuf(buf);
		}
	}
	else if(run_type == '<') {
		if(run_length > 127) {
			// sub ecx, imm32
			byte buf[] = { 0x81, 0xe9, run_length, run_length >> 8,
				run_length >> 16, run_length >> 24 };
			writebuf(buf);
		}
		else if(run_length > 1) {
			// sub ecx, imm8
			byte buf[] = { 0x83, 0xe9, run_length };
			writebuf(buf);
		}
		else if(run_length == 1) writebuf1(0x49); // dec ecx
		writebuf(left_wrap_asm);
	}
	else if(run_type == '>') {
		if(run_length > 127) {
			// add ecx, imm32
			byte buf[] = { 0x81, 0xc1, run_length, run_length >> 8,
				run_length >> 16, run_length >> 24 };
			writebuf(buf);
		}
		else if(run_length > 1) {
			// add ecx, imm8
			byte buf[] = { 0x83, 0xc1, run_length };
			writebuf(buf);
		}
		else if(run_length == 1) writebuf1(0x41); // inc ecx
		writebuf(right_wrap_asm);
	}
	run_length = 0;
	run_type = '\0';
}

int main(int argc, char** argv) {
	if(argc != 3)
		error("usage: %s <input> <output>\nuse \"-\" to read from stdin", argv[0]);
	FILE* inp = stdin;
	if(strcmp(argv[1], "-") != 0) {
		inp = fopen(argv[1], "r");
		if(!inp) error("error opening input file: %s", strerror(errno));
	}
	output = fopen(argv[2], "w");
	if(!output) error("error opening output file: %s", strerror(errno));
	chmod(argv[2], 0755);
	writebuf(header_bin);
	while(1) {
		char inp_buf[1024];
		int numread;
		if((numread = fread(inp_buf, 1, 1024, inp)) == 0) break;
		for(int i=0; i<numread; i++) {
			if(inp_buf[i] == '+' || inp_buf[i] == '-' ||
			   inp_buf[i] == '<' || inp_buf[i] == '>') {
				if(inp_buf[i] != run_type) end_run();
				run_type = inp_buf[i]; run_length++;
			}
			if(inp_buf[i] == '.') { end_run(); writejmpto(0xe8, header_sub_output); }
			if(inp_buf[i] == ',') { end_run(); writejmpto(0xe8, header_sub_input); }
			if(inp_buf[i] == '[') {
				end_run();
				if(loopdepth == sizeof(loopstack)/sizeof(int))
					error("error: too many nested loops");
				loopstack[loopdepth++] = out_off;
				writebuf(start_loop_asm);
			}
			if(inp_buf[i] == ']') {
				end_run();
				if(loopdepth == 0) error("error: too many closing brackets");
				uint32_t loop_tgt = loopstack[--loopdepth];
				uint32_t reloc_at = loop_tgt + sizeof(start_loop_asm) - 4;
				writejmpto(0xe9, loop_tgt);
				reloc32(reloc_at, out_off - (reloc_at+4));
			}
		}
	}
	end_run();
	if(loopdepth != 0) error("error: unclosed brackets");
	writebuf(fin_asm);
	uint32_t fsz = out_off;
	// twice because this is both the memory and file size
	reloc32(header_file_size_loc, fsz);
	reloc32(header_file_size_loc+4, fsz);

	reloc32(header_tape_size, 0x00100000);
	reloc32(header_tape_andmask, header_tape_addr + 0x00100000 - 1);
	
}
