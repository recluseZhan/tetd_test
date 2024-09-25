#!/bin/bash
sudo rmmod interrupt1
sudo rmmod cli1
make clean

make
sudo insmod cli1.ko
sudo insmod interrupt1.ko
echo "success\n"


