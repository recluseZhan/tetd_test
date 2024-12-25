#!/bin/bash
sudo rmmod log_dev1
sudo rm -rf /dev/log_dev
make clean

make
sudo insmod log_dev1.ko
sudo mknod /dev/log_dev c 233 0
sudo chmod 777 /dev/log_dev
echo "success\n"


