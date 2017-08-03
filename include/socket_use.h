/****************************************************************
 * Copyright(C), 2016, Hanbang Tech.Co.,Ltd
 * File Name:common.h
 * Author:merlin
 * Version:v1
 * Data:20160613
 * Description:һЩ���ýṹ��Ķ���
 * History:
 * *************************************************************/

#ifndef _SOCKET_USE_H_
#define _SOCKET_USE_H_

#include "common.h"

/*�������ӷ�ʽ*/
typedef enum
{
	KEEP_ALIVE, //������
	CLOSE, //������
} CONNECTION_E;

/*�������ӵ�������Ϣ*/
typedef struct _network_attr_S
{
	int fd; //�ļ�������
	int connect; //����״̬
	CONNECTION_E type; //���ӷ�ʽ
	time_t btime; //�������ӿ�ʼ��ʱ��
	char ip[64]; //IP��ַ
	int port; //�˿ں�	
	int complete; //���ͺ��Ƿ�����˽���
} NETWORK_ATTR_S, *PNETWORK_ATTR_S;

/*����״̬��Ϣ*/
typedef struct _connect_info{
	NETWORK_ATTR_S attr; /*��������*/
	pthread_mutex_t lock; /*������*/
}CONNECT_INFO, *P_CONNECT_INFO;

/*TCP���Գ���ʹ�õ��Ľṹ��*/
typedef struct _TCPDemoAttr{
	int fd;
	struct sockaddr_in addr;
}TCPDEMO_ATTR, *P_TCPDEMO_ATTR;


/*�׽��ֲ�����غ���*/
extern int bindSocket(const int fd, const int family, char *addr);
extern int bindSocket2(const int fd, const int family, char * addr, const int port);
extern int createSocket(const int family, const int type, const int protocol);
extern int listenSocket(const int fd, const int max);
//extern int acceptSocket(const int fd, const int family, char *addr, const int port);
extern int acceptSocket2(const int fd, struct sockaddr_in *clientAddr);
extern int demoTCPServerAndClient();
#endif

