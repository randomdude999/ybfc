#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bfc_common.h"
#include "arch_common.h"

size_t loopstack[256];
int loopdepth = 0;
size_t run_length = 0;
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
				if(loopdepth == sizeof(loopstack)/sizeof(size_t))
					error("error: too many nested loops");
				loopstack[loopdepth++] = out_off;
				write_start_loop();
			}
			if(inp_buf[i] == ']') {
				end_run();
				if(loopdepth == 0) error("error: too many closing brackets");
				size_t loop_tgt = loopstack[--loopdepth];
				write_end_loop(loop_tgt);
			}
		}
	}
}

int main(int argc, char** argv) {
	size_t tape_size = 0x8000;
	int opt;
	while ((opt = getopt(argc, argv, "t:o:m:")) != -1) {
		switch(opt) {
		case 'o':
			out_fname = strdup(optarg);
			break;
		case 't':
			errno = 0;
			char* endptr;
			long long tmp_tape = strtoll(optarg, &endptr, 10);
			if(errno == ERANGE || tmp_tape < 0 || *endptr != '\0')
				error("error: invalid tape size");
			tape_size = tmp_tape;
			if(tape_size & (tape_size-1))
				error("error: tape size must be a power of 2");
			break;
		case 'm':
			if(!strcmp(optarg, "i386")) current_arch = ARCH_i386;
			else if(!strcmp(optarg, "x64")) current_arch = ARCH_x64;
			else error("error: unknown architecture");
			break;
		default:
			error("usage: %s [-t tape_size] [-o output] [-m arch] <inputs>\n\n"
"Output defaults to a.out if not specified. Tape size must be a power of 2,\n"
"and not more than 1GiB (32TiB for x64). The default tape size is 32KiB.\n"
"Available architectures are i386 and x64.", argv[0]);
		}
	}
	if(tape_size > (current_arch == ARCH_i386 ? (1ll<<30) : (1ll<<45)))
		error("error: tape too large (maximum %s)", (current_arch == ARCH_i386 ? "1GiB" : "32TiB"));
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
