obj-m += aes_gcm.o
aes_gcm-objs := aes_gcm_mod.o aes_gcm_asm.o


all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

