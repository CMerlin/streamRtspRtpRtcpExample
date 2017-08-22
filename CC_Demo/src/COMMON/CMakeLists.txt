AUX_SOURCE_DIRECTORY(. DIR_TEST1_SRCS)
#include_directories(${DIR_TEST1_SRCS}../../include)
include_directories(../../include)
ADD_LIBRARY (libTRACE ${DIR_TEST1_SRCS})
