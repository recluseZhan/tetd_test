
obj-m += readgpa_dev1.o
readgpa_dev1-objs := readgpa_dev.o

obj-m += readgpa1.o
readgpa1-objs := readgpa.o

obj-m += hyper1.o
hyper1-objs := hyper.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
