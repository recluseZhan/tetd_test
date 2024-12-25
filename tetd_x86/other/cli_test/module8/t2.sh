#!/bin/bash
sudo rmmod sha2561
make clean

make
sudo insmod sha2561.ko
echo "success\n"


