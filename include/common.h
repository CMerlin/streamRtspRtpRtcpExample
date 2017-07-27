/****************************************************************
 * Copyright(C), 2016, Hanbang Tech.Co.,Ltd
 * File Name:common.h
 * Author:merlin
 * Version:v1
 * Data:20160613
 * Description:һЩ���ýṹ��Ķ���
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

/*�����������͵Ķ���*/
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

/* ģ��ID���� */
typedef enum
{
    COMP_SSOCKET        = 115,  /* �����׽��ֻ���ģ�� */
    COMP_NDCMPSRV       = 116,  /* �豸�������������ģ�� */
    COMP_DANALE         = 117,  /* ���ô�͸���� */
    COMP_MAX
} COMPID, *PCOMPID;

/*��־��ӡ����*/
typedef enum
{
	RELEASE,
	ERROR,
	TRACE,
	DEBUG,
} PRINT_LEVEL_E;

/*����RTSP����*/
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

/*RTSP��������ص���Ϣ*/
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
	int fd; /*�׽����ļ�*/
} RTSP_ATTR;

#define RX_BUF_LEN 1024
#define TX_BUG_LEN 1024

/*��װRTSP����֡��Ҫ�õ�����Ϣ*/
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
* Description:���ݰ���������Ϣ
*************************************************************************/
typedef struct _package_attr
{
	char head[32];
	char tail[32];
	char special[32];
	int len;
	int attr;
} PACKET_ATTR_S, *PPACKET_ATTR_S;

/*�շ�����֡���ص�����*/
typedef enum
{
	PACKET_ATTR_MIN_E,
	PACKET_ATTR_HEADANDTAIL_E, /*��ͷ��β*/
	PACKET_ATTR_HEAD_E, /*��ͷ��β*/
	PACKET_ATTR_LEN_E, /*�̶�����*/
	PACKET_ATTR_SPECIAL_E, /*�����ַ���*/
	PACKET_ATTR_TAIL_E, /*ֻ��β��*/
	PACKET_ATTR_MAX_E,
} PACKET_ATTR_E;

/*��־������غ���*/
extern int init_print_level();
extern int trace(const int plevel, const char *format, ...);
extern int initPrintAndPthread();
/*IO������صĺ���*/
extern sint32 read_data(const sint32 *file, void *data, sint32 size);
/*����vlc rtsp �������غ���*/
extern int parseVLCMsg(char *buffer, RTSP_MSG_ATTR *p_msgAttr);
extern int dealWirhRtspCmd(RTSP_ATTR *p_rtspAttr, RTSP_MSG_ATTR *p_msgAttr);
#endif

