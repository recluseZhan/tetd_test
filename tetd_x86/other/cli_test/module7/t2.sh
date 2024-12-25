#!/bin/bash
sudo rmmod rsa3
make clean

make
sudo insmod rsa3.ko
echo "success\n"


