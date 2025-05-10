CC = gcc
CFLAGS = -O3 -march=native -funroll-loops -fomit-frame-pointer -Wall
ASFLAGS = -O3 -march=native
LDFLAGS = 

OBJS = sign_main.o bignum.o sha256_asm_ni.o
TARGET = rsa

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

