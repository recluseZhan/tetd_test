# Makefile
CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS = -lcrypto
OBJS = rsa_sign.o bignum.o

all: rsa_sign

rsa_sign: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

rsa_sign.o: rsa_sign.c bignum.h
	$(CC) $(CFLAGS) -c rsa_sign.c

bignum.o: bignum.c bignum.h
	$(CC) $(CFLAGS) -c bignum.c

clean:
	rm -f rsa_sign *.o rsa_*.key

