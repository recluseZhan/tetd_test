C = gcc
CFLAGS = -O2 -Wall

OBJS = sign_main.o bignum.o sha256_asm_ni.o

all: rsa

rsa: $(OBJS)
	$(CC) -o $@ $(OBJS)

sign_main.o: sign_main.c bignum.h
	$(CC) $(CFLAGS) -c sign_main.c

bignum.o: bignum.c bignum.h error.h
	$(CC) $(CFLAGS) -c bignum.c

sha256_asm_ni.o: sha256_asm_ni.S
	$(CC) $(CFLAGS) -c sha256_asm_ni.S

clean:
	rm -f rsa *.o

