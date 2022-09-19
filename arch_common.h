#include <stddef.h>
#include <stdint.h>

#define DECL_ARCH_FUNC(NAME, SIGNATURE) \
	void arch_ ## NAME SIGNATURE; \
	void arch_i386_ ## NAME SIGNATURE; \
	void arch_x64_ ## NAME SIGNATURE;

// called before start of assembly
DECL_ARCH_FUNC(write_header, (size_t tape_size));
// called after all input has been assembled
DECL_ARCH_FUNC(end, ());

// called to write the asm corresponding to each command to the output
DECL_ARCH_FUNC(cmd_inc_run, (uint8_t count));
DECL_ARCH_FUNC(cmd_dec_run, (uint8_t count));
DECL_ARCH_FUNC(cmd_l_run, (size_t count));
DECL_ARCH_FUNC(cmd_r_run, (size_t count));
DECL_ARCH_FUNC(cmd_inp, ());
DECL_ARCH_FUNC(cmd_out, ());

// called for the loop start command
DECL_ARCH_FUNC(start_loop, ());
// called for the loop end command, argument is whatever the out_off was
// before the start_loop
DECL_ARCH_FUNC(end_loop, (size_t start_at));

// called after any command is emitted
DECL_ARCH_FUNC(post_cmd, ());

// should return the largest allowed tape size for the architecture
DECL_ARCH_FUNC(get_tape_max, (size_t* out));
