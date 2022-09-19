#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bfc_common.h"
#include "arch_common.h"

size_t* loopstack;
size_t loopstack_size = 256;
size_t loopdepth = 0;
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

static void push_loopstack(size_t val) {
	if(loopdepth == loopstack_size) {
		loopstack_size *= 2;
		loopstack = realloc(loopstack, loopstack_size*sizeof(size_t));
	}
	loopstack[loopdepth++] = val;
}

void end_run() {
	if(run_length == 0) return;
	if(run_type == '+') arch_cmd_inc_run(run_length);
	else if(run_type == '-') arch_cmd_dec_run(run_length);
	else if(run_type == '<') arch_cmd_l_run(run_length);
	else if(run_type == '>') arch_cmd_r_run(run_length);
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
			else if(inp_buf[i] == '.') { end_run(); arch_cmd_out(); }
			else if(inp_buf[i] == ',') { end_run(); arch_cmd_inp(); }
			else if(inp_buf[i] == '[') {
				end_run();
				push_loopstack(out_off);
				arch_start_loop();
			}
			else if(inp_buf[i] == ']') {
				end_run();
				if(loopdepth == 0) error("error: too many closing brackets");
				size_t loop_tgt = loopstack[--loopdepth];
				arch_end_loop(loop_tgt);
			}
		}
	}
}

int main(int argc, char** argv) {
	size_t tape_size = 0x8000;
	int opt;
	loopstack = malloc(loopstack_size*sizeof(size_t));
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
	size_t max_tape_size = 1<<30; // arch_get_tape_max(&max_tape_size);
	if(tape_size > max_tape_size)
		error("error: tape too large (maximum %zu)", max_tape_size);
	if(optind >= argc)
		error("error: no input files specified (use - to read from stdin)");
	output = fopen(out_fname, "w");
	if(!output) error("error opening output file: %s", strerror(errno));
	chmod(out_fname, 0755);

	arch_write_header(tape_size);

	for(; optind < argc; optind++) {
		process_file(argv[optind]);
	}

	end_run();
	if(loopdepth != 0) error("error: unclosed brackets");

	arch_end();
}
