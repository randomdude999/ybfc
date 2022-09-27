// SPDX-License-Identifier: ISC
#include <stddef.h>
#include <stdint.h>

#include "bfc_common.h"

#define PER_ARCH_DECL(ARCH, NAME, SIGNATURE, CALLSIG) \
	void arch_ ## ARCH ## _ ## NAME SIGNATURE;
#define PER_ARCH_SWITCH(ARCH, NAME, SIGNATURE, CALLSIG) \
	case ARCH_ ## ARCH: arch_ ## ARCH ## _ ## NAME CALLSIG; break;

#define FOR_ALL_ARCHES(FOR_ARCH, ...) \
	FOR_ARCH(i386, __VA_ARGS__) \
	FOR_ARCH(x64, __VA_ARGS__)

#define DECL_ARCH_FUNC(NAME, SIGNATURE, CALLSIG) \
	FOR_ALL_ARCHES(PER_ARCH_DECL, NAME, SIGNATURE, CALLSIG) \
	static void arch_ ## NAME SIGNATURE { switch (current_arch) { \
		FOR_ALL_ARCHES(PER_ARCH_SWITCH, NAME, SIGNATURE, CALLSIG) \
	}}

// called before start of assembly
DECL_ARCH_FUNC(write_header, (size_t tape_size), (tape_size));
// called after all input has been assembled
DECL_ARCH_FUNC(end, (), ());

// called to write the asm corresponding to each command to the output
DECL_ARCH_FUNC(cmd_inc_run, (uint8_t count), (count));
DECL_ARCH_FUNC(cmd_dec_run, (uint8_t count), (count));
DECL_ARCH_FUNC(cmd_l_run, (size_t count), (count));
DECL_ARCH_FUNC(cmd_r_run, (size_t count), (count));
DECL_ARCH_FUNC(cmd_inp, (), ());
DECL_ARCH_FUNC(cmd_out, (), ());

// called for the loop start command
DECL_ARCH_FUNC(start_loop, (), ());
// called for the loop end command, argument is whatever the out_off was
// before the start_loop
DECL_ARCH_FUNC(end_loop, (size_t start_at), (start_at));

// called after any command is emitted
DECL_ARCH_FUNC(post_cmd, (), ());

// should return the largest allowed tape size for the architecture
DECL_ARCH_FUNC(get_tape_max, (size_t* out), (out));

#undef PER_ARCH_DECL
#undef PER_ARCH_SWITCH
#undef FOR_ALL_ARCHES
#undef DECL_ARCH_FUNC
