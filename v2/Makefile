CFLAGS += -MMD

srcs = bfc.c arch_i386.c arch_common.c
objs = $(srcs:%.c=build/%.o)
deps = $(objs:.o=.d)

all: bfc

header.h: header.asm
	nasm -f bin -o header.bin $^
	xxd -i header.bin >$@
	cat header_sym.map | grep header_ | sed 's/\(.*\)  \(.*\)/uint32_t \2 = 0x\1;/' >>$@
	rm -f header.bin header_sym.map

build/arch_i386.o: header.h

build/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

bfc: $(objs)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f bfc $(objs) header.h

-include $(deps)
