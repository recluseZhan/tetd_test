cd ~/linux-intel-6.8.0/arch/x86/kvm

sudo make -C /lib/modules/`uname -r`/build M=`pwd` clean
sudo make -C /lib/modules/`uname -r`/build M=`pwd` modules
sudo modprobe -r kvm_intel
sudo modprobe -r kvm
modprobe irqbypass
sudo insmod kvm.ko
sudo insmod kvm-intel.ko

