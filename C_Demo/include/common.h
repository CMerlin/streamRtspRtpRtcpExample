/****************************************************************
 * Copyright(C), 2016, Hanbang Tech.Co.,Ltd
 * File Name:common.h
 * Author:merlin
 * Version:v1
 * Data:20160613
 * Description:一些共用结构体的定义
 * History:
 * *************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#if 1
#include <math.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#endif

/*基础数据类型的定义*/
typedef signed char         sint8;
typedef signed short        sint16;
typedef signed int          sint32;
typedef signed long         slng32;
typedef signed long long    sint64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long       ulng32;
typedef unsigned long long  uint64;

/* 模块ID定义 */
typedef enum
{
    COMP_SSOCKET        = 115,  /* 网络套接字基础模块 */
    COMP_NDCMPSRV       = 116,  /* 设备组网管理服务器模块 */
    COMP_DANALE         = 117,  /* 大拿穿透服务 */
    COMP_MAX
} COMPID, *PCOMPID;

/*日志打印级别*/
typedef enum
{
	RELEASE,
	ERROR,
	TRACE,
	DEBUG,
} PRINT_LEVEL_E;

/*各种RTSP命令*/
enum{
	RTSP_CMD_MIN,
	OPTION=1,
	DESCRIBE,
	SETUP,
	PLAY,
	PAUSE,
	TEARDOWN,
	ANNOUNCE,
	GET_PARAMETER,
	SET_PARAMETER,
	RTSP_CMD_MAX
};

/*RTSP服务器相关的信息*/
#define MIN_CLIENT_PORT_BIT     9
#define INVALID_SOCKET_VAL 			-1
#define TRUE		            1
#define FALSE		            0
#define RTSP_LISTEN_PORT        554// 
#define VIDEO_TRACKID 1
#define AUDIO_TRACKID 2

#define MAX_USER_NUM 3  /*最多支持的RTSP用户数量*/
#define MAX_CLIENT_NUM 64 /*最多支持的RTSP客户端数量*/

enum{
    ERR_CLIENT_MSG_PORT_TOO_SHORT=-100,    //port should larger than 9bit
};

typedef struct _netwokAttr{
	char ip[64];
	int port;
	int fd;
}NETWORK_ATTR, *P_NETWORK_ATTR;

typedef struct _rtspAttr{
	struct sockaddr_in addr;
	int fd; /*套接字文件*/
} RTSP_ATTR;

#define RX_BUF_LEN 1024
#define TX_BUG_LEN 1024

/*describe 消息包中的信息*/
typedef struct _describe{
	int CSeq;
	int timeout; /*时否超时处理*/
	char username[128];
	char realm[128];
	char nonce[12];
	char url[256];
	char response[128];
} SDESCRIBE, *P_SDESCRIBE;

typedef struct _useinfo{
	int enablePass; /*使能密码登陆功能*/
	char user[64]; /*用户名*/
	char pass[64]; /*密码*/
}USERINFO, *P_USERINFO;

/*RTSP服务器端的相关设置信息*/
typedef struct _rtsp_attr{
	pthread_mutex_t lock; /*互斥锁*/
	USERINFO userinfo[MAX_USER_NUM]; /*最多64个用户*/
}RTSP_ATTR_SERVER, *P_RTSP_ATTR_SERVER;

/*封装RTSP数据帧需要用到的信息*/
typedef struct _rtsp_msg_attr
{
	unsigned char *pmsg;
	int cmdType;
	unsigned int uiCseq;
	int iType;
	unsigned char aucCSeqSrc[10];
	unsigned char ucCSeqSrcLen;
	unsigned char aucTrans[200];
	unsigned char ucTransNum;
	unsigned char aucMrl[20];
	unsigned char ucMrlLen;
	unsigned char aucClientport[4][20];
	unsigned char ucClientportLen[4];
	unsigned char timeused;
	int roomid;//card 1/2/3/4
	int tcp_or_udp; //0 udp 1 tcp
	unsigned char aucClientInterleaved[4][20];
	unsigned char ucClientInterleavedLen[4];
}RTSP_MSG_ATTR, *P_RTSP_MSG_ATTR;

/***********************************************************************
* Description:数据包的属性信息
*************************************************************************/
typedef struct _package_attr
{
	char head[32];
	char tail[32];
	char special[32];
	int len;
	int attr;
} PACKET_ATTR_S, *PPACKET_ATTR_S;

/*收发数据帧的特点描述*/
typedef enum
{
	PACKET_ATTR_MIN_E,
	PACKET_ATTR_HEADANDTAIL_E, /*有头有尾*/
	PACKET_ATTR_HEAD_E, /*有头无尾*/
	PACKET_ATTR_LEN_E, /*固定长度*/
	PACKET_ATTR_SPECIAL_E, /*特殊字符串*/
	PACKET_ATTR_TAIL_E, /*只有尾巴*/
	PACKET_ATTR_MAX_E,
} PACKET_ATTR_E;

/*日志操作相关函数*/
extern int init_print_level();
extern int trace(const int plevel, const char *format, ...);
extern int initPrintAndPthread();
/*套接字操作相关函数*/
extern int bindSocket(const int fd, const int family, char *addr);
extern int bindSocket2(const int fd, const int family, char * addr, const int port);
extern int createSocket(const int family, const int type, const int protocol);
extern int listenSocket(const int fd, const int max);
//extern int acceptSocket(const int fd, const int family, char *addr, const int port);
extern int acceptSocket2(const int fd, struct sockaddr_in *clientAddr);
extern int demoTCPServerAndClient();

/*IO操作相关的函数*/
extern sint32 is_read_write(const sint32 *file);
extern sint32 read_data(const sint32 *file, void *data, sint32 size);
extern sint32 send_data(const sint32 *file, const void *data, const sint32 size);
/*解析vlc rtsp 命令的相关函数*/
extern int parseVLCMsg(char *buffer, RTSP_MSG_ATTR *p_msgAttr);
extern int dealWirhRtspCmd(RTSP_ATTR *p_rtspAttr, RTSP_MSG_ATTR *p_msgAttr);
/*RTSP操作相关函数*/
extern int initRTSPServerInfo();
extern int setRTSPServerInfo(USERINFO * attr);
extern int getRTSPServerInfo(USERINFO * attr);
#endif

