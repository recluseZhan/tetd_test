#!/bin/bash
sudo rmmod sha2562
make clean

make
sudo insmod sha2562.ko
echo "success\n"


