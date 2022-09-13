all: bfc bfrt.o

bfc: bfc.o
	$(CC) -o $@ $^

clean:
	rm -f bfc *.o

.PHONY: all clean
