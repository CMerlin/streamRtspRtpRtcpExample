/#!/bin/sh
echo "clean environment!"
rm ./rtsp_server_demo
killall -9 rtsp_server_demo
killall -9 server_rtsp_server_demo
killall -9 client_rtsp_server_demo

cp server_rtsp_server_demo ~/workDir/nfsDir/cl360/usr/local
cp client_rtsp_server_demo ~/workDir/nfsDir/cl360/usr/local
