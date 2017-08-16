/****************************************************************************************************
 * Description:�׽��ֵ���ز���
 * file: 
 * auht:merlin data:
 *****************************************************************************************************/
#include "common.h"
#include "socket_use.h"

/**
  int sock_init(int sockfd,struct sockaddr_in addr,int SERVER_PORT)
  {
  sockfd=socket(AF_INET,SOCK_DGRAM,0);

  if(sockfd<0)
  {
  fprintf(stderr,"Socket Error:%s\n",strerror(errno));
  exit(1);
  }
  printf("sockfd is %d\n",sockfd);
  bzero(&addr,sizeof(struct sockaddr_in));

  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=htonl(INADDR_ANY);
  addr.sin_port=htons(SERVER_PORT);

  if(bind(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))<0)
  {
  fprintf(stderr,"Bind Error:%s\n",strerror(errno));
  exit(1);
  }
  printf("init successfel!!\nport is %dsockfd is %d\n",SERVER_PORT,sockfd);
  return 0;
  }
  */
 
static CONNECT_INFO TCPClientConnectInfo; /*TCP�ͻ�������״̬��Ϣ*/
sint32 initTCPClientConnectInfo()
{
	pthread_mutex_init(&(TCPClientConnectInfo.lock), NULL); /*��ʼ��������*/
	memset(&(TCPClientConnectInfo.attr), 0, sizeof(TCPClientConnectInfo.attr));; /*��ʼ��������*/
	return 0;
}

/*******************************************************
 * Description������TCP�ͻ��˵�������Ϣ
 * Input attr����Ҫ���µ���Ϣ
 * Return��0-�ɹ��� -1-ʧ��
 * *****************************************************/
sint32 setNetAttrTCPC(NETWORK_ATTR_S *attr)
{
	pthread_mutex_lock(&(TCPClientConnectInfo.lock));
	TCPClientConnectInfo.attr = (*attr);
	pthread_mutex_unlock(&(TCPClientConnectInfo.lock));

	return 0;
}

/*******************************************************
 * Description����ȡTCP�ͻ��˵�������Ϣ
 * Output attr��������Ϣ
 * Return��0-�ɹ��� -1-ʧ��
 * *****************************************************/
sint32 getNetAttrTCPC(NETWORK_ATTR_S *attr)
{
	pthread_mutex_lock(&(TCPClientConnectInfo.lock));
	memset(attr, 0, sizeof(NETWORK_ATTR_S));
	(*attr) = TCPClientConnectInfo.attr;
	pthread_mutex_unlock(&(TCPClientConnectInfo.lock));

	return 0;
}

/************************************************************
* Description:����TCP�ͻ��˺ͷ������˵�����״̬
************************************************************/
sint32 setConnetSttatusTCPC(int status)
{
	pthread_mutex_lock(&(TCPClientConnectInfo.lock));
	TCPClientConnectInfo.attr.connect = status;
	pthread_mutex_unlock(&(TCPClientConnectInfo.lock));
	return 0;
}


/********************************************************************************
 * Description:��ȡ�����Ķ˿ں�
 *********************************************************************************/
int getLocalIP(char *zeroIP)
{
	//return -1;
	int i = 0;
	char hname[128] = {0}, *localIP = NULL;
	struct hostent *hent;
	if(NULL == zeroIP){
		printf("[%s]:in param is wrong! line:%d\n", __func__, __LINE__);
		return -1;
	}

	memset(zeroIP, 0, strlen(zeroIP));
	gethostname(hname, sizeof(hname));
	//hent = gethostent();
	hent = gethostbyname(hname);
	for(i = 0; hent->h_addr_list[i]; i++) {
		localIP = inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i]));
		if(NULL != localIP){
			memcpy(zeroIP, localIP, 64);
		}
		break;
		//printf("%s\n", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	}
#if 0
	trace(DEBUG, "[%s]:hostname: %s/naddress list: ", __func__, hent->h_name);
	for(i = 0; hent->h_addr_list[i]; i++) {
		printf("%s\n", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	}
	printf("\n");
#endif
	return 0;
}


/******************************************************************************
 * Description�������׽��� 
 * Input family��Э���� 
 * Input type��Э������ 
 * Input protocol��
 * Return�������õ��ļ�������
 * *****************************************************************************/
int createSocket(const int family, const int type, const int protocol)
{ 
	int fd = socket(family, type, protocol);	
	if(fd < 0)	{	
		close(fd);
		printf("[%s]:socket=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		//trace(ERROR, "[%s]:socket=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return fd;
	}
	return fd;
}
/****************************************************************************** 
 * Description�����׽��� 
 * Input fd����Ҫ�󶨵��ļ� 
 * Input family��Э������ 
 * Input addr����ַ
 * Return��fd	
 * *****************************************************************************/
int bindSocket(const int fd, const int family, char *addr) 
{	
	int len = 0;	
	struct sockaddr_un un;	
	unlink(addr);	
	memset(&un, 0, sizeof(un));
	un.sun_family = family;
	strcpy(un.sun_path, addr);	
	len = offsetof(struct sockaddr_un, sun_path) + strlen(addr);
	if(bind(fd, (struct sockaddr *)&un, len) < 0)
	{		
		printf("[%s]:bind=%s line:%d\n", __func__, strerror(errno), __LINE__); 
		return -1;
	}	
	return fd;
}

int bindSocket2(const int fd, const int family, char * addr, const int port)
{
	int result = 0;
	struct sockaddr_in bindAddr;
	
	bzero(&bindAddr,sizeof(struct sockaddr_in));
	//addr.sin_family=AF_INET;
	bindAddr.sin_family=family;
	//addr.sin_addr.s_addr=htonl(INADDR_ANY);
	//char serverIP[64] = {"192.168.5.192"};
	//memcpy((addr.sin_addr.s_addr), serverIP, strlen(serverIP) );
	inet_pton(AF_INET, addr, &(bindAddr.sin_addr));
	bindAddr.sin_port=htons(port);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &result, sizeof(result)); //���bind
	if(bind(fd,(struct sockaddr *)&bindAddr,sizeof(struct sockaddr_in))<0)
	{
		trace(ERROR, "[%s]:bind=%s line:%d\n", __func__, strerror(errno), __LINE__); 
		return -1;
	}
	return 0;
}

/******************************************************************************
 * Description�������׽��� 
 * Input fd��
 * Input max��listen���ɼ������ļ���
 * Return���������ļ�
 * *****************************************************************************/
int listenSocket(const int fd, const int max)
{	
	if(listen(fd, max) < 0) 
	{	
		printf("[%s]:listen=%s line:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	return fd;
}


/******************************************************************************
 * Description��������������
 * Input��
 * Input��
 * Output�� 
 * Return���ѽ����յ��ļ������� 
 * *****************************************************************************/
int acceptSocket(const int fd, const int family, char *addr, const int port) //TCP	UDP
{	
	socklen_t len = 0;
	int cfd = 0;
	struct sockaddr_in un;
	un.sin_family = family;
	//un.sin_addr.sin_addr.s_addr = addr;
	//un.sin_addr.sin_addr.s_addr = INADDR_ANY;
	un.sin_port = htons(port);	
	len = sizeof(struct sockaddr_in);
	cfd = accept(fd, (struct sockaddr *)&un, &len);
	if(cfd < 0)
	{		
		printf("[%s]:accept=%s line:%d\n", __func__, strerror(errno), __LINE__);	
		return -1;	
	}	
	return cfd;
}

/********************************************************************************
* Description:���տͻ��˵���������
*
*********************************************************************************/
int acceptSocket2(const int fd, struct sockaddr_in *clientAddr)
{
	int ret = 0;
	int len = sizeof(struct sockaddr_in);
	memset(clientAddr, 0, sizeof(struct sockaddr_in));
	ret = accept(fd, (struct sockaddr *)clientAddr, (socklen_t*)&len);
	if(0 > ret){
		printf("[%s]:accept=%s line:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	
	trace(DEBUG, "[%s]:client IP=%s port=%d socket=%d line=%d\n", __func__, inet_ntoa(clientAddr->sin_addr), (int)ntohs(clientAddr->sin_port), ret, __LINE__);
	//nslog(NS_INFO, "[runRtspDebug][ID:%lld]:line:IP=%s port=%d line=%d\n", ttid2, inet_ntoa(clientAddr.sin_addr), (int)ntohs(clientAddr.sin_port), __LINE__);

	return ret;
}

/****************************************************************************** 
 * Description������ͷ������˽�������
 * Input��
 * Output��
 * Return�� 
 * *****************************************************************************/
int connectSocket(int *fd, const int family, char *ip, const int port) //TCP UDP
{	
	struct sockaddr_in addr;
	addr.sin_family = family;
	//un.sin_addr.sin_addr.s_addr = addr;
	//inet_pton(AF_INET, ip, addr.sin_addr.sin_addr.s_addr);
	inet_pton(AF_INET, ip, &(addr.sin_addr));
	addr.sin_port = htons(port);
	//if(connect(*fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
	if(0 > connect(*fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
	{		
		close(*fd); 
		printf("[%s]:connect=%s LINE:%d\n", __func__, strerror(errno),__LINE__);		
		//trace(ERROR, "[%s]:connect=%s LINE:%d\n", __func__, strerror(errno),__LINE__);		
		return -1;	
	}	
	return 0;
}
/****************************************************************************************************/

/******************************************************************************
 * Description���ͷ������������ӣ����ӷ�ʽ��������
 * Input ip and port���������˵�ip�Ͷ˿�
 * Output fd and complete���׽����ļ���������һЩ������ص���Ϣ
 * Return��0-�ɹ� -1-ʧ��
 * *****************************************************************************/
sint32 retry_connect_server_block(NETWORK_ATTR_S *server)
{
	sint32 ret = 0;
	NETWORK_ATTR_S history;
	
	getNetAttrTCPC(&history);
	//trace(DEBUG, "[%s]:history connect info fd=%d ip=%s port=%d connect=%d type=%d LINE:%d\n", __func__, history.fd, history.ip, history.port, history.connect, history.type, __LINE__);
	/*���������˿ڵ�IP�Ƿ����˸ı�*/
	if(0 != strncmp(history.ip, server->ip, strlen(history.ip))){
		history.connect = 0;
		close(history.fd);
	}
	/*����Ƕ����ӣ������µ�����*/
	if(server->type != KEEP_ALIVE){
		history.connect = 0;
	}
	/*�����������ٽ������Ӳ���*/
	if(history.connect == 1){
		(*server) = history;
		return 0;
	}

	/*�����쳣�����µ�����*/
	if(server->fd > 0){
		close(server->fd);
		server->fd = 0;
	}
	/*�����׽���*/
	server->btime = time(NULL);
	server->fd = createSocket(AF_INET, SOCK_STREAM, 0);
	if(server->fd < 0){
		trace(ERROR, "[%s]:createSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
#if 0 /*bind�˿ڵĲ���, �ͻ�����ñ�bind�˿�*/
	int cliPort = 9009;
	bindSocket2((server->fd), AF_INET, (server->ip), cliPort);
#endif
	/*����������*/
	ret = connectSocket(&(server->fd), AF_INET, server->ip, server->port);
	if(ret < 0){
		trace(ERROR, "[%s]:connectSocket excuate faild! ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
		return -1;
	}
	trace(TRACE, "[%s]:succed try connect to server->ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
	/*���óɹ��������ӵı�־*/
	server->connect = 1;
	memset(&history, 0, sizeof(history));
	history = (*server);
	setNetAttrTCPC(&history);

	return 0;
}


char* sock_recv(int sockfd,struct sockaddr *addr_client,int *addrlen)
{
	//int *tem_len = &addrlen;
	socklen_t len;
	printf("[%s]:sock_recv sockfd is %d line:%d\n", __func__, sockfd, __LINE__);
	char *recv_buffer = malloc (sizeof (char));
	//printf("sock_recv sockfd is88888888888888 %d\n",sockfd);
	int n;
	n=recvfrom(sockfd,recv_buffer,256,0,addr_client,&len);
	printf("[%s]:recv number is %d line:%d\n", __func__, n, __LINE__);
	if(0)
	{
		printf("[%s]:recvfrom error! line:%d\n", __func__, __LINE__);
		exit (1);
	}
	if(-1==n)
	{
		printf("[%s]:recv error: line:%d\n", __func__, __LINE__);
	}
	else
	{
		addrlen=(int *)len;
		printf("[%s]:sock recv success!! line:%d\n", __func__, __LINE__);
		/**   char IPdotdec[20]; //��ŵ��ʮ����IP��ַ
		  struct in_addr s =
		  inet_ntop(AF_INET, (void *)&s, IPdotdec, 16);
		  printf("addr_client.data=%s\n",IPdotdec);
		  */ printf("[%s]:addr_len=%d line:%d\n", __func__, *addrlen, __LINE__);
	}
	return recv_buffer;
}

/****************************************************************************************
* Description:�����̣߳����߳���Դ���л���
*****************************************************************************************/
int myPthreadExit()
{
	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return 0;
}

/********************************************************************************************
* Description:��������,�������ݽ��л���
********************************************************************************************/
void dealWithData(void *inParam)
{
	int timeOut = 0;
	char buffer[1024] = {0};
	int ret = 0, bufLen = 256;
	TCPDEMO_ATTR clientAttr;
	
	if(NULL == inParam){
		trace(ERROR, "[%s]:in param is wrong! line:%d\n", __func__, __LINE__);
		myPthreadExit();
		return;
	}

	memcpy(&clientAttr, inParam, sizeof(TCPDEMO_ATTR));
	timeOut = time(NULL);
	while(1){
		/*�ܾû�û���յ��ͻ��˿ڵ����ݣ��ͽ���*/
		if(10 <= (time(NULL)-timeOut)){
			trace(DEBUG, "[%s]:timeout close client! line:%d\n", __func__, __LINE__);
			myPthreadExit();
			close(clientAttr.fd);
		}
		/*�շ�����*/
		memset(buffer, 0, strlen(buffer));
		ret = read_data(&(clientAttr.fd), buffer, bufLen);
		if(0 >= ret){
			//trace(ERROR, "[%s]:ret=%d read failure! line:%d\n", __func__, ret, __LINE__);
			//usleep(1*1000*1000); /*����3���ڶ�*/
			continue;
		}
		timeOut = time(NULL);
		trace(DEBUG, "[%s]:recv[%d]:%s line:%d\n", __func__, ret, buffer, __LINE__);
		if(0 >= send_data(&(clientAttr.fd), buffer, strlen(buffer))){
			trace(ERROR, "[%s]:send failure! line:%d\n", __func__, __LINE__);
		}
	}
	
	return;
}

/*****************************************************************************************
 * Description:�׽��ֵĽӿڲ��Ժ���,TCP��������
 *******************************************************************************************/
int serverTCP()
{
	pthread_t pthreadID = 0;
	TCPDEMO_ATTR ClientAttr;
	struct sockaddr_in clientAddr;
	sint32 sfd = 0, ret = 0, port = 9002, cliSocket = -1;
	char ip[64] = {"192.168.5.130"};

	trace(DEBUG, "[%s]:begin line:%d\n", __func__, __LINE__);
	/*��ȡ����ip*/
	char localIP[64] = {0};
	if(0 > getLocalIP(localIP)){
		trace(ERROR, "[%s]:get local ip failed! line:%d\n", __func__, __LINE__);
	}else{
		memset(ip, 0, strlen(ip));
		memcpy(ip, localIP, strlen(localIP));
	}

	trace(DEBUG, "[%s]:local IP=%s port=%d line:%d\n", __func__, ip, port, __LINE__);
	/*�����׽���*/
	sfd= createSocket(AF_INET, SOCK_STREAM, 0);
	if(0 >= sfd){
		trace(ERROR, "[%s]:create socket failure! line:%d\n", __func__, __LINE__);
		return -1;
	}
	trace(DEBUG, "[%s]:local----------------  line:%d\n", __func__, __LINE__);
	/*bind�׽���*/
	bindSocket2(sfd, AF_INET, ip, port);
	/*�����׽���*/
	if(listenSocket(sfd, 3) < 0){
		trace(ERROR, "[%s]:listen timeout! line:%d\n", __func__, __LINE__);
		return -1;
	}
	
	while(1){
		trace(DEBUG, "[%s]:local----------------  line:%d\n", __func__, __LINE__);
		/*������������*/
		memset(&clientAddr, 0, sizeof(clientAddr));
		cliSocket = acceptSocket2(sfd, &clientAddr);
		if(0 >= cliSocket){
			usleep(3*1000*1000);
			trace(ERROR, "[%s][Error]:accept failure re=%d line:%d\n", __func__, ret, __LINE__);
			continue;
			//sleep(5);
		}
		trace(DEBUG, "[%s]:new conenct socket=%d *********************** line:%d\n", __func__, cliSocket, __LINE__);
		memset(&ClientAttr, 0, sizeof(ClientAttr));
		memcpy(&(ClientAttr.addr), &clientAddr, sizeof(clientAddr));
		ClientAttr.fd = cliSocket;
		ret = pthread_create(&pthreadID, NULL, (void *)dealWithData, (void *)&ClientAttr);
		if(ret < 0){
			trace(ERROR, "[%s]:create pthread failure! line:%d\n", __func__, __LINE__);
		}
	}

	return 0;
}

/*****************************************************************************************
 * Description:�׽��ֵĽӿڲ��Ժ���,TCP�ͻ���
 *******************************************************************************************/
int clientTCP()
{
	NETWORK_ATTR_S serverNetAttr; /*TCP�������˵�������Ϣ*/
	int ret = 0, bufLen = 256;
	char ip[64] = {"192.168.5.95"};
	char buffer[256] = {"hello I'm is client!"};

	trace(DEBUG, "[%s]:begin line:%d\n", __func__, __LINE__);
	/*��ȡ����ip*/
	char localIP[64] = {0};
	if(0 > getLocalIP(localIP)){
		trace(ERROR, "[%s]:get local ip failed! line:%d\n", __func__, __LINE__);
	}else{
		memset(ip, 0, strlen(ip));
		memcpy(ip, localIP, strlen(localIP));
	}
	/*����TCP�������˵�������Ϣ*/
	memset(&serverNetAttr, 0, sizeof(serverNetAttr));
	memcpy((serverNetAttr.ip), ip, strlen(ip));
	serverNetAttr.port = 9002;
	serverNetAttr.type = KEEP_ALIVE;
	initTCPClientConnectInfo();
	setNetAttrTCPC(&serverNetAttr);
	
	/*�շ�������*/
	while(1){
		usleep(2*1000*1000);
		/*��������Ƿ�����,�쳣�����´�������*/
		if(0 > retry_connect_server_block(&serverNetAttr)){
			trace(ERROR, "[%s]:create connect failure! line:%d\n", __func__, __LINE__);
		} 
		/*�շ�������*/
		ret = send_data(&(serverNetAttr.fd), buffer, strlen(buffer));
		if(0 >= ret){
			serverNetAttr.connect = -1;
			setConnetSttatusTCPC(serverNetAttr.connect);
			trace(ERROR, "[%s]:send data failure! line:%d\n", __func__, __LINE__);
			continue;
		}
		trace(DEBUG, "[%s]:send[%d]:%s line:%d\n", __func__, ret, buffer, __LINE__);
		usleep(1*1000*1000);
		//memset(buffer, 0, strlen(buffer));
		ret = read_data(&(serverNetAttr.fd), buffer, bufLen);
		if(0 >= ret){
			trace(ERROR, "[%s]:read data failure! line:%d\n", __func__, __LINE__);
			continue;
		}
		trace(DEBUG, "[%s]:recv[%d]:%s line:%d\n", __func__, ret, buffer, __LINE__);
	}

	return 0;
}

/**********************************************************************************
* Description:����һ�����̣������ͻ��˳���
**********************************************************************************/
int startTCPServer()
{
	//trace(DEBUG, "[%s]:start line:%d\n", __func__, __LINE__);
	pid_t pid1 = fork();
	if(0 < pid1){
		//sleep(1);
		//exit(0);
	}
	else if(0 == pid1){
		//trace(DEBUG, "[%s]:start TCP server line:%d\n", __func__, __LINE__);
		pid_t pid2 = fork();
		if(0 < pid2){
			sleep(1);
			exit(0);
		}
		else if(0 == pid2){
			serverTCP(); //����TCP��������
		}
		else{
			trace(ERROR, "[%s]:fork=%s line:%d\n", __func__, strerror(errno), __LINE__);
		}
	}
	else{
		trace(ERROR, "[%s]:fork=%s line:%d\n", __func__, strerror(errno), __LINE__);
	}
	
	return 0;
}

/*********************************************************************************
* Description:����һ�����̣������������˳���
***********************************************************************************/
int startTCPClient()
{
	pid_t pid1 = fork();
	if(0 < pid1){
		//sleep(1);
		//exit(0);
	}
	else if(0 == pid1){
		//trace(DEBUG, "[%s]:start TCP server line:%d\n", __func__, __LINE__);
		pid_t pid2 = fork();
		if(0 < pid2){
			sleep(1);
			exit(0);
		}
		else if(0 == pid2){
			clientTCP(); //����TCP�ͻ���
		}
		else{
			trace(ERROR, "[%s]:fork=%s line:%d\n", __func__, strerror(errno), __LINE__);
		}
	}
	else{
		trace(ERROR, "[%s]:fork=%s line:%d\n", __func__, strerror(errno), __LINE__);
	}
	
	return 0;
}

/*********************************************************************************
* Desccription:TCP�������˺Ϳͻ��˳�����Գ���
************************************************************************************/
int demoTCPServerAndClient()
{
	initPrintAndPthread(); /*��ʼ����ӡ������쳣�źŴ���*/
	//serverTCP();
	//clientTCP();
	startTCPServer(); /*������������*/
	sleep(3);
	startTCPClient(); /*�����ͻ��˶�*/

	return 0;
}

