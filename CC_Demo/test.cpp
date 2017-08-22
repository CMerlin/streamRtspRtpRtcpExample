#include "print.h"
#if 0
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
//using std::vector;
#endif
typedef unsigned char uint8_t;
typedef signed char int8_t;

#if 1 /*RTP数据包header的解析*/
static int parsingRTPPacket(unsigned char *data, int size) {  
	if (size < 12) {  
        return -1;  
    }  
	unsigned int version = (data[0] >> 6); /*版本*/   
    unsigned int numCSRCs = data[0] & 0x0f;  
    unsigned int payloadOffset = 12 + 4 * numCSRCs;    
    unsigned int rtpTime = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7]; /*时间戳*/
    unsigned int srcId = data[8] << 24 | data[9] << 16 | data[10] << 8 | data[11];  
    unsigned int seqNum = data[2] << 8 | data[3];  
	printf("[%s]:seqNum=%d version=%d numCSRCs=%d payloadOffset=%d rtpTime=%d srcId=%d line:%d\n", __func__, seqNum, version, numCSRCs, payloadOffset, rtpTime, srcId, __LINE__);
		
    return 0;  
}  
#endif

int main(int argc, const char *argv[])
{
#if 0
	/*输出的目标字符串前面会加上系统时间*/
	const int TIME = 1;
	printInfo print(USERD, TIME);
	print.print(USERD, "Hello!\n");
	//输出结果为：2015082415:08:35.789772 Hello!
#endif

#if 1 /*获取进程和线程号*/
	char buffer[65] = {0};
	//pid_t pid = getpid();
	//sprintf(id, "%d.%lu", pid, pthread_self());
	sprintf(buffer, "%d.%lu", getpid(), pthread_self());
	//printf("[%s]:id=%s line:%d\n", __func__, id, __LINE__);
	std::cout<<"buffer="<<buffer<<std::endl;
#endif

	parsingRTPPacket((unsigned char*)buffer, 12);

	return 0;
}
