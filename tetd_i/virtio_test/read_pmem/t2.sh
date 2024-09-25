#!/bin/bash
sudo rmmod pmem_read1
make clean

make
sudo insmod pmem_read1.ko
echo "success\n"
