#!/bin/bash

cd ..
mv output output.old

cd buildroot
mv output output.old

cd ..
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme build
./build-scripts/aemfvp-a-rme/build-test-buildroot.sh -p aemfvp-a-rme package

