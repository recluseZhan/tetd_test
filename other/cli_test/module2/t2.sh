#!/bin/bash
sudo rmmod cli1
make clean

make
sudo insmod cli1.ko
echo "success\n"


