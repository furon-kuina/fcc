CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

fcc: $(OBJS)
				$(CC) -o fcc $(OBJS) $(LDFLAGS)

$(OBJS): fcc.h

test: fcc
				./test.sh

clean:
				rm -f fcc *.o *~ tmp*

.PHONY: test clean