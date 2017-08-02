#!/bin/sh
echo "clean environment!"
rm ./rtsp_server_demo
killall -9 rtsp_server_demo

echo "step 1. make"
rm -rf build
mkdir build
cd build
cmake ..
make
cd ..
cp ./build/bin/rtsp_server_demo ./
