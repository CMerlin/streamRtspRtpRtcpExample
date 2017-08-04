/****************************************************************************************************
 * Description:套接字的相关操作
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
 
static CONNECT_INFO TCPClientConnectInfo; /*TCP客户端连接状态信息*/
sint32 initTCPClientConnectInfo()
{
	pthread_mutex_init(&(TCPClientConnectInfo.lock), NULL); /*初始化互斥锁*/
	memset(&(TCPClientConnectInfo.attr), 0, sizeof(TCPClientConnectInfo.attr));; /*初始化数据区*/
	return 0;
}

/*******************************************************
 * Description：更新TCP客户端的网络信息
 * Input attr：需要更新的信息
 * Return：0-成功； -1-失败
 * *****************************************************/
sint32 setNetAttrTCPC(NETWORK_ATTR_S *attr)
{
	pthread_mutex_lock(&(TCPClientConnectInfo.lock));
	TCPClientConnectInfo.attr = (*attr);
	pthread_mutex_unlock(&(TCPClientConnectInfo.lock));

	return 0;
}

/*******************************************************
 * Description：获取TCP客户端的网络信息
 * Output attr：连接信息
 * Return：0-成功； -1-失败
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
* Description:设置TCP客户端和服务器端的连接状态
************************************************************/
sint32 setConnetSttatusTCPC(int status)
{
	pthread_mutex_lock(&(TCPClientConnectInfo.lock));
	TCPClientConnectInfo.attr.connect = status;
	pthread_mutex_unlock(&(TCPClientConnectInfo.lock));
	return 0;
}


/********************************************************************************
 * Description:获取本机的端口号
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
 * Description：创建套接字 
 * Input family：协议族 
 * Input type：协议类型 
 * Input protocol：
 * Return：创建好的文件描述符
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
 * Description：绑定套接字 
 * Input fd：需要绑定的文件 
 * Input family：协议类型 
 * Input addr：地址
 * Return：fd	
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
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &result, sizeof(result)); //解除bind
	if(bind(fd,(struct sockaddr *)&bindAddr,sizeof(struct sockaddr_in))<0)
	{
		trace(ERROR, "[%s]:bind=%s line:%d\n", __func__, strerror(errno), __LINE__); 
		return -1;
	}
	return 0;
}

/******************************************************************************
 * Description：监听套接字 
 * Input fd：
 * Input max：listen最多可监听的文件数
 * Return：监听的文件
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
 * Description：接受连接请求
 * Input：
 * Input：
 * Output： 
 * Return：已将接收的文件描述符 
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
* Description:接收客户端的连接请求
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
 * Description：申请和服务器端建立连接
 * Input：
 * Output：
 * Return： 
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
 * Description：和服务器建立连接，连接方式是阻塞的
 * Input ip and port：服务器端的ip和端口
 * Output fd and complete：套接字文件描述符和一些连接相关的信息
 * Return：0-成功 -1-失败
 * *****************************************************************************/
sint32 retry_connect_server_block(NETWORK_ATTR_S *server)
{
	sint32 ret = 0;
	NETWORK_ATTR_S history;
	
	getNetAttrTCPC(&history);
	//trace(DEBUG, "[%s]:history connect info fd=%d ip=%s port=%d connect=%d type=%d LINE:%d\n", __func__, history.fd, history.ip, history.port, history.connect, history.type, __LINE__);
	/*检测服务器端口的IP是否发生了改变*/
	if(0 != strncmp(history.ip, server->ip, strlen(history.ip))){
		history.connect = 0;
		close(history.fd);
	}
	/*如果是短连接，建立新的连接*/
	if(server->type != KEEP_ALIVE){
		history.connect = 0;
	}
	/*连接正常不再进行连接操作*/
	if(history.connect == 1){
		(*server) = history;
		return 0;
	}

	/*连接异常建立新的连接*/
	if(server->fd > 0){
		close(server->fd);
		server->fd = 0;
	}
	/*创建套接字*/
	server->btime = time(NULL);
	server->fd = createSocket(AF_INET, SOCK_STREAM, 0);
	if(server->fd < 0){
		trace(ERROR, "[%s]:createSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
#if 0 /*bind端口的操作, 客户端最好别bind端口*/
	int cliPort = 9009;
	bindSocket2((server->fd), AF_INET, (server->ip), cliPort);
#endif
	/*请求建立连接*/
	ret = connectSocket(&(server->fd), AF_INET, server->ip, server->port);
	if(ret < 0){
		trace(ERROR, "[%s]:connectSocket excuate faild! ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
		return -1;
	}
	trace(TRACE, "[%s]:succed try connect to server->ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
	/*设置成功建立连接的标志*/
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
		/**   char IPdotdec[20]; //存放点分十进制IP地址
		  struct in_addr s =
		  inet_ntop(AF_INET, (void *)&s, IPdotdec, 16);
		  printf("addr_client.data=%s\n",IPdotdec);
		  */ printf("[%s]:addr_len=%d line:%d\n", __func__, *addrlen, __LINE__);
	}
	return recv_buffer;
}

/****************************************************************************************
* Description:结束线程，对线程资源进行回收
*****************************************************************************************/
int myPthreadExit()
{
	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return 0;
}

/********************************************************************************************
* Description:接收数据,并将数据进行回射
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
		/*很久还没有收到客户端口的数据，就结束*/
		if(10 <= (time(NULL)-timeOut)){
			trace(DEBUG, "[%s]:timeout close client! line:%d\n", __func__, __LINE__);
			myPthreadExit();
			close(clientAttr.fd);
		}
		/*收发数据*/
		memset(buffer, 0, strlen(buffer));
		ret = read_data(&(clientAttr.fd), buffer, bufLen);
		if(0 >= ret){
			//trace(ERROR, "[%s]:ret=%d read failure! line:%d\n", __func__, ret, __LINE__);
			//usleep(1*1000*1000); /*休眠3秒在读*/
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
 * Description:套接字的接口测试函数,TCP服务器端
 *******************************************************************************************/
int serverTCP()
{
	pthread_t pthreadID = 0;
	TCPDEMO_ATTR ClientAttr;
	struct sockaddr_in clientAddr;
	sint32 sfd = 0, ret = 0, port = 9002, cliSocket = -1;
	char ip[64] = {"192.168.5.130"};

	trace(DEBUG, "[%s]:begin line:%d\n", __func__, __LINE__);
	/*获取本机ip*/
	char localIP[64] = {0};
	if(0 > getLocalIP(localIP)){
		trace(ERROR, "[%s]:get local ip failed! line:%d\n", __func__, __LINE__);
	}else{
		memset(ip, 0, strlen(ip));
		memcpy(ip, localIP, strlen(localIP));
	}

	trace(DEBUG, "[%s]:local IP=%s port=%d line:%d\n", __func__, ip, port, __LINE__);
	/*创建套接字*/
	sfd= createSocket(AF_INET, SOCK_STREAM, 0);
	if(0 >= sfd){
		trace(ERROR, "[%s]:create socket failure! line:%d\n", __func__, __LINE__);
		return -1;
	}
	trace(DEBUG, "[%s]:local----------------  line:%d\n", __func__, __LINE__);
	/*bind套接字*/
	bindSocket2(sfd, AF_INET, ip, port);
	/*监听套接字*/
	if(listenSocket(sfd, 3) < 0){
		trace(ERROR, "[%s]:listen timeout! line:%d\n", __func__, __LINE__);
		return -1;
	}
	
	while(1){
		trace(DEBUG, "[%s]:local----------------  line:%d\n", __func__, __LINE__);
		/*接收连接请求*/
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
 * Description:套接字的接口测试函数,TCP客户端
 *******************************************************************************************/
int clientTCP()
{
	NETWORK_ATTR_S serverNetAttr; /*TCP服务器端的网络信息*/
	int ret = 0, bufLen = 256;
	char ip[64] = {"192.168.5.95"};
	char buffer[256] = {"hello I'm is client!"};

	trace(DEBUG, "[%s]:begin line:%d\n", __func__, __LINE__);
	/*获取本机ip*/
	char localIP[64] = {0};
	if(0 > getLocalIP(localIP)){
		trace(ERROR, "[%s]:get local ip failed! line:%d\n", __func__, __LINE__);
	}else{
		memset(ip, 0, strlen(ip));
		memcpy(ip, localIP, strlen(localIP));
	}
	/*设置TCP服务器端的网络信息*/
	memset(&serverNetAttr, 0, sizeof(serverNetAttr));
	memcpy((serverNetAttr.ip), ip, strlen(ip));
	serverNetAttr.port = 9002;
	serverNetAttr.type = KEEP_ALIVE;
	initTCPClientConnectInfo();
	setNetAttrTCPC(&serverNetAttr);
	
	/*收发送数据*/
	while(1){
		usleep(2*1000*1000);
		/*检测连接是否正常,异常就重新创建连接*/
		if(0 > retry_connect_server_block(&serverNetAttr)){
			trace(ERROR, "[%s]:create connect failure! line:%d\n", __func__, __LINE__);
		} 
		/*收发送数据*/
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
* Description:创建一个进程，启动客户端程序
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
			serverTCP(); //启动TCP服务器端
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
* Description:创建一个进程，启动服务器端程序
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
			clientTCP(); //启动TCP客户端
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
* Desccription:TCP服务器端和客户端程序测试程序
************************************************************************************/
int demoTCPServerAndClient()
{
	initPrintAndPthread(); /*初始化打印级别和异常信号处理*/
	//serverTCP();
	//clientTCP();
	startTCPServer(); /*启动服务器端*/
	sleep(3);
	startTCPClient(); /*启动客户端端*/

	return 0;
}

