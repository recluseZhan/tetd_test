#!/bin/bash
sudo rmmod readgpa_dev1
sudo rmmod readgpa1
sudo rm -rf /dev/readgpa_dev
sudo make clean

sudo make
sudo insmod readgpa1.ko
sudo insmod readgpa_dev1.ko
sudo mknod /dev/readgpa_dev c 232 0
sudo chmod 777 /dev/readgpa_dev
echo "success\n"
