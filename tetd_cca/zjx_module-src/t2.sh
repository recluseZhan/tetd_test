#!/bin/bash

cd ../buildroot
make zjx_module-dirclean
make zjx_module
make

cd ..
#./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme build
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme package
