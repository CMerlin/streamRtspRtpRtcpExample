/****************************************************************
 * Copyright(C), 2016, Hanbang Tech.Co.,Ltd
 * File Name:common.h
 * Author:merlin
 * Version:v1
 * Data:20160613
 * Description:一些共用结构体的定义
 * History:
 * *************************************************************/

#ifndef _SOCKET_USE_H_
#define _SOCKET_USE_H_

#include "common.h"

/*网络连接方式*/
typedef enum
{
	KEEP_ALIVE, //长连接
	CLOSE, //短连接
} CONNECTION_E;

/*用于连接的网络信息*/
typedef struct _network_attr_S
{
	int fd; //文件描述符
	int connect; //连接状态
	CONNECTION_E type; //连接方式
	time_t btime; //建立连接开始的时间
	char ip[64]; //IP地址
	int port; //端口号	
	int complete; //发送和是否完成了接受
} NETWORK_ATTR_S, *PNETWORK_ATTR_S;

/*连接状态信息*/
typedef struct _connect_info{
	NETWORK_ATTR_S attr; /*网络属性*/
	pthread_mutex_t lock; /*互斥锁*/
}CONNECT_INFO, *P_CONNECT_INFO;

/*TCP测试程序使用到的结构体*/
typedef struct _TCPDemoAttr{
	int fd;
	struct sockaddr_in addr;
}TCPDEMO_ATTR, *P_TCPDEMO_ATTR;


/*套接字操作相关函数*/
extern int bindSocket(const int fd, const int family, char *addr);
extern int bindSocket2(const int fd, const int family, char * addr, const int port);
extern int createSocket(const int family, const int type, const int protocol);
extern int listenSocket(const int fd, const int max);
//extern int acceptSocket(const int fd, const int family, char *addr, const int port);
extern int acceptSocket2(const int fd, struct sockaddr_in *clientAddr);
extern int demoTCPServerAndClient();
#endif

