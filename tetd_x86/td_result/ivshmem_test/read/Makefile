obj-m += ivshmem_read1.o
ivshmem_read1-objs := ivshmem_read.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
