CC = gcc
CFLAGS = -O2 -Wall

# 一定要包含 asn1.o，否则链接不了 pem_read_der / asn1_parse_***
OBJS = bignum.o asn1.o rsa_pem_sign.o

all: rsa_pem_sign

rsa_pem_sign: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

bignum.o: bignum.c bignum.h
	$(CC) $(CFLAGS) -c bignum.c

asn1.o: asn1.c asn1.h bignum.h
	$(CC) $(CFLAGS) -c asn1.c

rsa_pem_sign.o: rsa_pem_sign.c bignum.h asn1.h
	$(CC) $(CFLAGS) -c rsa_pem_sign.c

clean:
	rm -f rsa_pem_sign *.o

