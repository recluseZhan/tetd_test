obj-m += write1.o
write1-objs := write.o

obj-m += read1.o
read1-objs := read.o

obj-m += tdi_dev1.o
tdi_dev1-objs := tdi_dev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

