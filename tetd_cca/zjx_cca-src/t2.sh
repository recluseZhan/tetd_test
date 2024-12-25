#!/bin/bash

cd ../buildroot
make zjx_cca-dirclean
make zjx_cca
make

cd ..
#./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme build
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme package
