#!/bin/bash

cd ..
rm -rf output.old
mv output output.old

cd buildroot
rm -rf output.old
mv output output.old

cd ..
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme build
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme package

