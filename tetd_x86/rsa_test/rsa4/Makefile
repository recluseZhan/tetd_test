CC = gcc
CFLAGS = -O2 -Wall

all: rsa_embedded_key

rsa_embedded_key: bignum.o rsa_embedded_key.o
	$(CC) -o $@ $^

bignum.o: bignum.c bignum.h
	$(CC) $(CFLAGS) -c bignum.c

rsa_embedded_key.o: rsa_embedded_key.c bignum.h
	$(CC) $(CFLAGS) -c rsa_embedded_key.c

clean:
	rm -f rsa_embedded_key *.o

