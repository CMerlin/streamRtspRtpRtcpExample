#!/bin/sh

echo "step 1. make"
rm -rf build
mkdir build
cd build
cmake ..
make
cd ..
echo "step 2. run test program"
cp build/bin/printInfo ./
