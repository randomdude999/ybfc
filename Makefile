srcs = ybfc.c arch_i386.c arch_x64.c arch_win32.c
objs = $(srcs:%.c=build/%.o)
deps = $(objs:.o=.d)

all: ybfc

build/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

ybfc: $(objs)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f ybfc $(objs) $(deps)

hw.elf: bfc
	./ybfc -m x64 t/hw.b -o $@

-include $(deps)
