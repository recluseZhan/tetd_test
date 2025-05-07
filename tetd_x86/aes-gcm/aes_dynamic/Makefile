obj-m += aes_gcm.o
aes_gcm-objs := aes_gcm_asm.o aes_gcm_module.o

ccflags-y += -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -masm=att

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

