#!/bin/bash
sudo rmmod ivshmem_read1
make clean

make
sudo insmod ivshmem_read1.ko
echo "success\n"
