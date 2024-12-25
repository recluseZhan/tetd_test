sudo rmmod tdi_dev1
sudo rmmod read1
sudo rmmod write1
sudo rm -rf /dev/tdi_dev
make clean

make
sudo insmod write1.ko
sudo insmod read1.ko
sudo insmod tdi_dev1.ko
sudo mknod /dev/tdi_dev c 233 0
sudo chmod 777 /dev/tdi_dev
