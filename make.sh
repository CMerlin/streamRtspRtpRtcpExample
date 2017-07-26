#!/bin/sh

echo "step 1. make"
rm -rf build
mkdir build
cd build
cmake ..
make
cd ..
cp ./build/bin/rtsp_server_demo ./
