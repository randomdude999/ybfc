#include "arch_common.h"
#include "bfc_common.h"

#define delegate(FUNC, SIGNATURE, CALLSIG) void arch_ ## FUNC SIGNATURE { \
	switch(current_arch) { \
		case ARCH_i386: arch_i386_ ## FUNC CALLSIG; break; \
		case ARCH_x64: arch_x64_ ## FUNC CALLSIG; break; \
	} }

delegate(write_header, (size_t tape_size), (tape_size));
delegate(end, (), ());
delegate(cmd_inc_run, (uint8_t count), (count));
delegate(cmd_dec_run, (uint8_t count), (count));
delegate(cmd_l_run, (size_t count), (count));
delegate(cmd_r_run, (size_t count), (count));
delegate(cmd_inp, (), ());
delegate(cmd_out, (), ());
delegate(start_loop, (), ());
delegate(end_loop, (size_t start_at), (start_at));
delegate(post_cmd, (), ());
delegate(get_tape_max, (size_t* out), (out));