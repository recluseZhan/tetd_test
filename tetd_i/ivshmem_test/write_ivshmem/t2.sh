#!/bin/bash
sudo rmmod ivshmem_write1
make clean

make
sudo insmod ivshmem_write1.ko
echo "success\n"
