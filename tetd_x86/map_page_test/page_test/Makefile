# Makefile for manual_remap_fullpt.ko

obj-m := manual_remap_fullpt.o

KDIR   := /lib/modules/$(shell uname -r)/build
PWD    := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

