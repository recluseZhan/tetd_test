obj-m += ivshmem_write1.o
ivshmem_write1-objs := ivshmem_write.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
