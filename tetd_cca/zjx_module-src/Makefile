CROSS_COMPILE ?= /home/ubuntu/rme-stack/buildroot/output/host/bin/aarch64-linux-
KERNEL_DIR = /home/ubuntu/rme-stack/linux/lib/modules/6.7.0-rc4-g19ff5f60db62-dirty/build
PWD := $(shell pwd)

obj-m += set_ipa1.o
set_ipa1-objs := set_ipa.o

obj-m += get_ipa1.o
get_ipa1-objs := get_ipa.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) clean

