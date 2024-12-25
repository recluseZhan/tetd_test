obj-m += interrupt1.o
interrupt1-objs := interrupt.o

obj-m += cli1.o
cli1-objs := cli.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

