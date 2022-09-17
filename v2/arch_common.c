#include "arch_common.h"
#include "bfc_common.h"
#include "arch_i386.h"

#define delegate(FUNC, SIGNATURE, CALLSIG) void write_ ## FUNC SIGNATURE { \
	switch(current_arch) { \
		case ARCH_i386: write_i386_ ## FUNC CALLSIG; break; \
		case ARCH_x64: error("architecture not implemented"); \
	} }

delegate(header, (size_t tape_size), (tape_size));
delegate(end, (), ());
delegate(cmd_inc_run, (uint8_t count), (count));
delegate(cmd_dec_run, (uint8_t count), (count));
delegate(cmd_l_run, (size_t count), (count));
delegate(cmd_r_run, (size_t count), (count));
delegate(cmd_inp, (), ());
delegate(cmd_out, (), ());
delegate(start_loop, (), ());
delegate(end_loop, (size_t start_at), (start_at));
