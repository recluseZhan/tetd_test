obj-m += aes_gcm.o
aes_gcm-objs := aes_gcm_mod.o aesni_ecb.o


all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

