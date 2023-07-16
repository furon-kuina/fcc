CFLAGS=-std=c11 -g -static

fcc: fcc.c

test: fcc
				./test.sh

clean:
				rm -f fcc *.o *~ tmp*

.PHONY: test clean