#!/bin/bash
sudo rmmod tdi_dev1
sudo rmmod Malware
sudo rm -rf /dev/tdi_dev
make clean

make
sudo insmod Malware.ko
sudo insmod tdi_dev1.ko
sudo mknod /dev/tdi_dev c 233 0
sudo chmod 777 /dev/tdi_dev
echo "success\n"
