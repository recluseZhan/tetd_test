OBJs = test

CFLAGS = -Wall -O2

all: $(OBJs)

test: bignum.o rsa.o test.o
	$(CC) $(CFLAGS) -o $@ $^ -static

bignum.o: bignum.c bignum.h
	$(CC) $(CFLAGS) -c bignum.c

rsa.o: rsa.c rsa.h
	$(CC) $(CFLAGS) -c rsa.c

test.o: test.c bignum.h rsa.h
	$(CC) $(CFLAGS) -c test.c

.PHONY: clean run

run:
	time ./test

clean:
	- rm $(OBJs) *.o
