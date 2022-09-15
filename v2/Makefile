all: bfc

header.h: header.asm
	nasm -f bin -o header.bin $^
	xxd -i header.bin >$@
	cat header_sym.map | grep header_ | sed 's/\(.*\)  \(.*\)/uint32_t \2 = 0x\1;/' >>$@
	rm -f header.bin header_sym.map

bfc.o: header.h

bfc: bfc.o 

clean:
	rm -f bfc bfc.o header.h
