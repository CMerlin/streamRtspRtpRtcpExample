PROJECT(main)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#设定交叉编译工具
#set(CMAKE_C_COMPILER arm-linux-g++)
set(CMAKE_C_COMPILER g++)
#可执行程序存放路径
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ./bin)
#编译的选项
set(CMAKE_C_FLAGS "-Wall -g")
#支持c++11
add_compile_options(-std=c++11)
ADD_SUBDIRECTORY( src )
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
#可执行程序的名称
ADD_EXECUTABLE(test_RTSP ${DIR_SRCS}  )
#头包含头文件
include_directories(${PROJECT_SOURCE_DIR}/include)
#测试程序依赖的库
TARGET_LINK_LIBRARIES( test_RTSP libTRACE libRTSP)
