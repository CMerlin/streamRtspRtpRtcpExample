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
/*IO操作相关的函数*/
extern sint32 read_data(const sint32 *file, void *data, sint32 size);
/*解析vlc rtsp 命令的相关函数*/
extern int parseVLCMsg(char *buffer, RTSP_MSG_ATTR *p_msgAttr);
extern int dealWirhRtspCmd(RTSP_ATTR *p_rtspAttr, RTSP_MSG_ATTR *p_msgAttr);
#endif

