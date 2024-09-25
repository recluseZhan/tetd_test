#!/bin/bash
sudo rmmod pmem_write1
make clean

make
sudo insmod pmem_write1.ko
echo "success\n"
