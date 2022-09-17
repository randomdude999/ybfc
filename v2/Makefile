CFLAGS += -MMD

srcs = bfc.c arch_i386.c arch_common.c
objs = $(srcs:.c=.o)
deps = $(srcs:.c=.d)

all: bfc

header.h: header.asm
	nasm -f bin -o header.bin $^
	xxd -i header.bin >$@
	cat header_sym.map | grep header_ | sed 's/\(.*\)  \(.*\)/uint32_t \2 = 0x\1;/' >>$@
	rm -f header.bin header_sym.map

bfc: bfc.o arch_i386.o arch_common.o

clean:
	rm -f bfc $(objs) header.h

-include $(deps)
