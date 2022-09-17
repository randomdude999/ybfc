#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bfc_common.h"
#include "arch_common.h"

uint32_t loopstack[256];
int loopdepth = 0;
int run_length = 0;
char run_type;
const char* out_fname = "a.out";
FILE* output;
size_t out_off;
int current_arch = ARCH_i386;

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
static void writejmpto(byte opc, uint32_t target) {
	uint32_t offset = target - (out_off+5);
	byte buf[5] = {opc, offset, offset >> 8, offset >> 16, offset >> 24};
	writebuf(buf);
}

void reloc32(size_t reloc_at, uint32_t data) {
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
	if(run_type == '+') write_cmd_inc_run(run_length);
	else if(run_type == '-') write_cmd_dec_run(run_length);
	else if(run_type == '<') write_cmd_l_run(run_length);
	else if(run_type == '>') write_cmd_r_run(run_length);
	run_length = 0;
	run_type = '\0';
}

void process_file(char * fname) {
	FILE* inp = stdin;
	if(strcmp(fname, "-") != 0) {
		inp = fopen(fname, "r");
		if(!inp) error("error opening input file: %s", strerror(errno));
	}
	while(1) {
		char inp_buf[1024];
		int numread;
		// check this manually to avoid having to press ^D twice
		// when manually typing input
		if(feof(inp)) break;
		if((numread = fread(inp_buf, 1, 1024, inp)) == 0) break;
		for(int i=0; i<numread; i++) {
			if(inp_buf[i] == '+' || inp_buf[i] == '-' ||
			   inp_buf[i] == '<' || inp_buf[i] == '>') {
				if(inp_buf[i] != run_type) end_run();
				run_type = inp_buf[i]; run_length++;
			}
			if(inp_buf[i] == '.') { end_run(); write_cmd_out(); }
			if(inp_buf[i] == ',') { end_run(); write_cmd_inp(); }
			if(inp_buf[i] == '[') {
				end_run();
				if(loopdepth == sizeof(loopstack)/sizeof(int))
					error("error: too many nested loops");
				loopstack[loopdepth++] = out_off;
				write_start_loop();
			}
			if(inp_buf[i] == ']') {
				end_run();
				if(loopdepth == 0) error("error: too many closing brackets");
				uint32_t loop_tgt = loopstack[--loopdepth];
				write_end_loop(loop_tgt);
			}
		}
	}

}

int main(int argc, char** argv) {
	uint32_t tape_size = 0x8000;
	int opt;
	while ((opt = getopt(argc, argv, "t:o:")) != -1) {
		switch(opt) {
		case 'o':
			out_fname = strdup(optarg);
			break;
		case 't':
			errno = 0;
			long tmp_tape = strtol(optarg, NULL, 10);
			if(errno || tmp_tape < 0 || tmp_tape > UINT32_MAX)
				error("error: invalid tape size");
			tape_size = tmp_tape;
			if(tape_size & (tape_size-1))
				error("error: tape size must be a power of 2");
			if(tape_size > (1ll<<30))
				error("error: tape too large (maximum 1GiB)");
			break;
		default:
			error("usage: %s [-t tape_size] [-o output] <inputs>\n\n"
"Output defaults to a.out if not specified. Tape size must be a power of 2,\n"
"and not more than 1GiB. The default tape size is 32KiB.", argv[0]);
		}
	}
	if(optind >= argc)
		error("error: no input files specified (use - to read from stdin)");
	output = fopen(out_fname, "w");
	if(!output) error("error opening output file: %s", strerror(errno));
	chmod(out_fname, 0755);

	write_header(tape_size);

	for(; optind < argc; optind++) {
		process_file(argv[optind]);
	}

	end_run();
	if(loopdepth != 0) error("error: unclosed brackets");

	write_end();
}
