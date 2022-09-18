#include <stddef.h>
#include <stdint.h>

void write_header(size_t tape_size);
void write_end();

// write the asm corresponding to the command to the output file
void write_cmd_inc_run(uint8_t count);
void write_cmd_dec_run(uint8_t count);
void write_cmd_l_run(size_t count);
void write_cmd_r_run(size_t count);

void write_cmd_inp();
void write_cmd_out();

void write_start_loop();
// argument is whatever the file offset was before calling start_loop
void write_end_loop(size_t start_at);

#define DECL_ARCH_FUNC(NAME, SIGNATURE) \
	void write_i386_ ## NAME SIGNATURE; \
	void write_x64_ ## NAME SIGNATURE;

DECL_ARCH_FUNC(header, (size_t tape_size));
DECL_ARCH_FUNC(end, ());
DECL_ARCH_FUNC(cmd_inc_run, (uint8_t count));
DECL_ARCH_FUNC(cmd_dec_run, (uint8_t count));
DECL_ARCH_FUNC(cmd_l_run, (size_t count));
DECL_ARCH_FUNC(cmd_r_run, (size_t count));
DECL_ARCH_FUNC(cmd_inp, ());
DECL_ARCH_FUNC(cmd_out, ());
DECL_ARCH_FUNC(start_loop, ());
DECL_ARCH_FUNC(end_loop, (size_t start_at));
