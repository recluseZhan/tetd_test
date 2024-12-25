obj-m += log1.o
log1-objs := log.o

obj-m += tdi_dev1.o
tdi_dev1-objs := tdi_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
