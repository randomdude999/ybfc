#include <stddef.h>
#include <stdint.h>

void write_i386_header(size_t tape_size);
void write_i386_end();

// write the asm corresponding to the command to the output file
void write_i386_cmd_inc_run(uint8_t count);
void write_i386_cmd_dec_run(uint8_t count);
void write_i386_cmd_l_run(size_t count);
void write_i386_cmd_r_run(size_t count);

void write_i386_cmd_inp();
void write_i386_cmd_out();

void write_i386_start_loop();
void write_i386_end_loop(size_t start_at);
