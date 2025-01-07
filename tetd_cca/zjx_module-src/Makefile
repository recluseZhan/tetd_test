CROSS_COMPILE ?= /home/ubuntu/rme-stack/buildroot/output/host/bin/aarch64-linux-
KERNEL_DIR = /home/ubuntu/rme-stack/linux/lib/modules/6.7.0-rc4-g19ff5f60db62-dirty/build
PWD := $(shell pwd)

obj-m += set_ipa1_1.o
set_ipa1_1-objs := set_ipa1.o

obj-m += set_ipa2_1.o
set_ipa2_1-objs := set_ipa2.o

obj-m += set_ripas1.o
set_ripas1-objs := set_ripas.o

obj-m += get_ipa1.o
get_ipa1-objs := get_ipa.o

obj-m += destroy_undelegate_ipa1.o
destroy_undelegate_ipa1-objs := destroy_undelegate_ipa.o

obj-m += destroy_ipa1.o
destroy_ipa1-objs := destroy_ipa.o

obj-m += block1.o
block1-objs := block.o

obj-m += unblock1.o
unblock1-objs := unblock.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) clean

