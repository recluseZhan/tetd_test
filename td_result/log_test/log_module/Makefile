KDIR := /root/log_test/log_module

obj-m += log_module1.o
log_module1-objs := log_module.o

#obj-m += tdi_dev1.o
#tdi_dev1-objs := tdi_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules EXTRA_CFLAGS="-I$(KDIR)"
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
