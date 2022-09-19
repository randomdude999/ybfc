srcs = ybfc.c arch_i386.c arch_common.c arch_x64.c
objs = $(srcs:%.c=build/%.o)
deps = $(objs:.o=.d)

all: ybfc

build/header_i386.h build/header_x64.h: build/header_%.h: header_%.asm
	nasm -f bin -o header.bin $^
	echo "static uint8_t header_bin[] = {" >$@
	cat header.bin | xxd -i >>$@
	echo "};" >>$@
	cat header_sym.map | grep export_ | sed 's/\(.*\)  export_\(.*\)/static const size_t header_\2 = 0x\1;/' >>$@
	rm header.bin
	mv header_sym.map $@.map

build/arch_i386.o: build/header_i386.h
build/arch_x64.o: build/header_x64.h

build/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -Ibuild -c $< -o $@

ybfc: $(objs)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f ybfc $(objs) $(deps) build/header_*.h

hw.elf: bfc
	./ybfc -m x64 t/hw.b -o $@

-include $(deps)
