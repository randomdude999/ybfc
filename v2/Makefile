all: bfc

header.bin: header.asm
	nasm -f bin -o $@ $^

header.h: header.bin
	xxd -i $^ >$@
	cat header_sym.map | grep header_ | sed 's/\(.*\)  \(.*\)/uint32_t \2 = 0x\1;/' >>$@

bfc.o: header.h

bfc: bfc.o 
	$(CC) -o $@ $^

clean:
	rm -f bfc bfc.o header.bin header.h header_sym.map
