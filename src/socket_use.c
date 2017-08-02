/****************************************************************************************************
 * Description:�׽��ֵ���ز���
 * file: 
 * auht:merlin data:
 *****************************************************************************************************/
#include "common.h"

typedef struct _TCPDemoAttr{
	int fd;
	struct sockaddr_in addr;
}TCPDEMO_ATTR, *P_TCPDEMO_ATTR;

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
  
/********************************************************************************
 * Description:��ȡ�����Ķ˿ں�
 *********************************************************************************/
int getLocalIP(char *zeroIP)
{
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

/********************************************************************************************
* Description:�������˿ڴ���һ���߳����շ�����
**********************************************************************************************/
void dealWithData(void *inParam)
{
	if(NULL == inParam){
		trace(ERROR, "[%s]:in param is wrong! line:%d\n", __func__, __LINE__);
		pthread_detach(pthread_self());
		pthread_exit(NULL);
		return;
	}
	
	int bufLen = 256;
	char buffer[256] = {0};
	TCPDEMO_ATTR *pAttr = inParam;
	int ret = 0, cliSocket = pAttr->fd;

	trace(DEBUG, "[%s]:cliSocket=%d line:%d\n", __func__, cliSocket, __LINE__);
	while(1){
		sleep(2);
		/*�շ�������*/
		trace(DEBUG, "[%s]:file status=%d line:%d\n", __func__, is_read_write(&cliSocket), __LINE__);
#if 1
		memset(buffer, 0, strlen(buffer));
		ret = read_data(&cliSocket, buffer, bufLen);
		if(0 < ret){
			trace(DEBUG, "[%s]:recv[%s]:%s line:%d\n", __func__, ret, buffer, __LINE__);
		}
		ret = send_data(&cliSocket, buffer, strlen(buffer)); 
#endif
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
	char ip[64] = {"192.168.5.199"};

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
		return -1;
	}

	/*bind�׽���*/
	bindSocket2(sfd, AF_INET, ip, port);
	/*�����׽���*/
	if(listenSocket(sfd, 3) < 0){
		trace(ERROR, "[%s]:listen timeout! line:%d\n", __func__, __LINE__);
		return -1;
	}

	while(1){
		/*������������*/
		memset(&clientAddr, 0, sizeof(clientAddr));
		cliSocket = acceptSocket2(sfd, &clientAddr);
		if(0 > cliSocket){
			continue;
			trace(ERROR, "[%s][Error]:accept failure re=%d line:%d\n", __func__, ret, __LINE__);
			//sleep(5);
		}
		memset(&ClientAttr, 0, sizeof(ClientAttr));
		memcpy(&(ClientAttr.addr), &clientAddr, sizeof(clientAddr));
		ClientAttr.fd = cliSocket;
		pthread_create(&pthreadID, NULL, (void *)dealWithData, (void *)&ClientAttr);
#if 0
		/*�շ�������*/
		trace(DEBUG, "[%s]:cliSocket=%d line:%d\n", __func__, cliSocket, __LINE__);
		memset(buffer, 0, strlen(buffer));
		ret = read_data(&cliSocket, buffer, bufLen);
		if(0 < ret){
			trace(DEBUG, "[%s]:recv[%s]:%s line:%d\n", __func__, ret, buffer, __LINE__);
		}
		ret = send_data(&cliSocket, buffer, strlen(buffer)); 
#endif
		//sleep(3);
	}

	return 0;
}

/*****************************************************************************************
 * Description:�׽��ֵĽӿڲ��Ժ���,TCP�ͻ���
 *******************************************************************************************/
int clientTCP()
{
	int ret = 0, bufLen = 256, connection = -1;
	sint32 sfd = 0, port = 9002; /*�������˵�IP*/
	char ip[64] = {"192.168.5.199"};
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

	trace(DEBUG, "[%s]:local IP=%s port=%d line:%d\n", __func__, ip, port, __LINE__);
	/*�����׽���*/
	sfd= createSocket(AF_INET, SOCK_STREAM, 0);
	if(0 >= sfd){
		return -1;
	}
	/*����ͷ������˽�������*/
	ret = connectSocket(&sfd, AF_INET, ip, port);
	if(ret < 0){
		trace(ERROR, "[%s]:connectSocket excuate faild! ip=%s port=%d LINE:%d\n", __func__, ip, port, __LINE__);
	}
	/*�շ�������*/
	while(1){
		/*������Ӷ��ˣ��ط�����������*/
		if(0 >= connection){
			sfd= createSocket(AF_INET, SOCK_STREAM, 0);
			if(0 >= sfd){
				continue;
			}
			connection = 1;
		}
		/*�շ�������*/
		trace(DEBUG, "[%s]:file status=%d file=%d line:%d\n", __func__, is_read_write(&sfd), sfd, __LINE__);
		ret = send_data(&sfd, buffer, strlen(buffer));
		if(0 < ret){
			connection = 0;
			trace(DEBUG, "[%s]:send[%d]:%s line:%d\n", __func__, ret, buffer, __LINE__);
		}
		memset(buffer, 0, strlen(buffer));
		ret = read_data(&sfd, buffer, bufLen);
		if(0 >= ret){
			connection = 0;
		}
		sleep(3);
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
* Description:����һ�����̣������������˿�
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
	return 0;
}


/*********************************************************************************
* Desccription:TCP�������˺Ϳͻ��˳�����Գ���
************************************************************************************/
int demoTCPServerAndClient()
{
	initPrintAndPthread(); /*��ʼ����ӡ������쳣�źŴ���*/
	trace(DEBUG, "[%s]:start line:%d\n", __func__, __LINE__);
	startTCPServer(); /*������������*/
	sleep(3);
	startTCPClient(); /*�����ͻ��˶�*/
	return 0;
}

