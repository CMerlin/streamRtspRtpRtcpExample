PROJECT(rmu)

cmake_minimum_required(VERSION 2.6)
#set(CMAKE_C_COMPILER /home/julyrain/toolschain/arm-2008q1/bin/arm-linux-gcc)
set(CMAKE_C_COMPILER arm-linux-gcc)
#set(CMAKE_C_COMPILER arm-linux-g++)
#set(CMAKE_C_COMPILER arm-none-linux-gnueabi-gcc)
#设置交叉编译工具
set(CMAKE_C_COMPILER gcc)
#可执行程序存放路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#编译的选项
set(CMAKE_C_FLAGS "-Wall -g")
#允许c++11标准 O3优化 多线程
set(CMAKE_C_FLAGS "-pthread")
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread")

#add_definitions(-DUSING_TIMERFD)
#add_definitions(-DUSING_ZIP)

#头文件
include_directories(${PROJECT_SOURCE_DIR}/include)
#${PROJECT_SOURCE_DIR}/components/include
	#				${PROJECT_SOURCE_DIR}/utilities/tools)
#.CC文件
add_subdirectory(src)
#add_subdirectory(utilities)
