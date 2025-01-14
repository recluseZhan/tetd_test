#!/bin/bash

cd ../linux

export CROSS_COMPILE=/home/zjx/rme-stack/buildroot/output/host/bin/aarch64-linux-

make ARCH=arm64 prepare
make ARCH=arm64 -j$(nproc)
make ARCH=arm64 modules -j$(nproc)
sudo make ARCH=arm64 CROSS_COMPILE=$CROSS_COMPILE INSTALL_MOD_PATH=/home/zjx/rme-stack/linux modules_install

cd ..
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme package
