#!/bin/bash
sudo rmmod page1
make clean

make
sudo insmod page1.ko
echo "success\n"


