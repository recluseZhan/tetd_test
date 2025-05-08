CC = gcc
CFLAGS = -O2 -Wall
OBJS = bignum.o rsa_sign.o

all: rsa_sign

rsa_sign: $(OBJS)
	$(CC) -o $@ $(OBJS)

bignum.o: bignum.c bignum.h
	$(CC) $(CFLAGS) -c bignum.c

rsa_sign.o: rsa_sign.c bignum.h
	$(CC) $(CFLAGS) -c rsa_sign.c

clean:
	rm -f rsa_sign *.o rsa_*.key

