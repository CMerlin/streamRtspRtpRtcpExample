// NALDecoder.cpp : Defines the entry point for the console application.
//

#include "common.h"
#include "h264.h"

//#define DEST_PORT            8888

typedef struct
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx
	char *buf;                    //! contains the first byte followed by the EBSP
	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;

FILE *bits = NULL;                //!< the bit stream file
static int FindStartCode2 (unsigned char *Buf);//查找开始字符0x000001
static int FindStartCode3 (unsigned char *Buf);//查找开始字符0x00000001
//static bool flag = true;
static int info2=0, info3=0;
RTP_FIXED_HEADER        *rtp_hdr;

NALU_HEADER		*nalu_hdr;
FU_INDICATOR	*fu_ind;
FU_HEADER		*fu_hdr;
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

#if 1
/****************************************************************************************************
 * Description:套接字的相关操作
 *****************************************************************************************************/
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
	struct sockaddr_in bindAddr;
	
	bzero(&bindAddr,sizeof(struct sockaddr_in));
	//addr.sin_family=AF_INET;
	bindAddr.sin_family=family;
	//addr.sin_addr.s_addr=htonl(INADDR_ANY);
	//char serverIP[64] = {"192.168.5.192"};
	//memcpy((addr.sin_addr.s_addr), serverIP, strlen(serverIP) );
	inet_pton(AF_INET, addr, &(bindAddr.sin_addr));
	bindAddr.sin_port=htons(port);
	if(bind(fd,(struct sockaddr *)&bindAddr,sizeof(struct sockaddr_in))<0)
	{
		printf("[%s]:bind=%s line:%d\n", __func__, strerror(errno), __LINE__); 
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
#endif


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

//为NALU_t结构体分配内存空间
NALU_t *AllocNALU(int buffersize)
{
	NALU_t *n;

	if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
	{
		printf("AllocNALU: n");
		exit(0);
	}

	n->max_size=buffersize;

	if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
	{
		free (n);
		printf ("AllocNALU: n->buf");
		exit(0);
	}

	return n;
}
//释放
void FreeNALU(NALU_t *n)
{
	if (n)
	{
		if (n->buf)
		{
			free(n->buf);
			n->buf=NULL;
		}
		free (n);
	}
}

void OpenBitstreamFile (char *fn)
{
	if (NULL == (bits=fopen(fn, "rb")))
	{
		printf("open file error\n");
		exit(0);
	}
	printf("test264 open successful!\n");
}
//这个函数输入为一个NAL结构体，主要功能为得到一个完整的NALU并保存在NALU_t的buf中，获取他的长度，填充F,IDC,TYPE位。
//并且返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
int GetAnnexbNALU (NALU_t *nalu)
{
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf;

	if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
		printf ("GetAnnexbNALU: Could not allocate Buf memory\n");

	nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节

	if (3 != fread (Buf, 1, 3, bits))//从码流中读3个字节
	{
		free(Buf);
		return 0;
	}
	info2 = FindStartCode2 (Buf);//判断是否为0x000001
	if(info2 != 1)
	{
		//如果不是，再读一个字节
		if(1 != fread(Buf+3, 1, 1, bits))//读一个字节
		{
			free(Buf);
			return 0;
		}
		info3 = FindStartCode3 (Buf);//判断是否为0x00000001
		if (info3 != 1)//如果不是，返回-1
		{
			free(Buf);
			return -1;
		}
		else
		{
			//如果是0x00000001,得到开始前缀为4个字节
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}

	else
	{
		//如果是0x000001,得到开始前缀为3个字节
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}
	//查找下一个开始字符的标志位
	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

	while (!StartCodeFound)
	{
		if (feof (bits))//判断是否到了文件尾
		{
			nalu->len = (pos-1)-nalu->startcodeprefix_len;
			memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			free(Buf);
			return pos-1;
		}
		Buf[pos++] = fgetc (bits);//读一个字节到BUF中
		info3 = FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001
		if(info3 != 1)
			info2 = FindStartCode2(&Buf[pos-3]);//判断是否为0x000001
		StartCodeFound = (info2 == 1 || info3 == 1);
	}



	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;

	if (0 != fseek (bits, rewind, SEEK_CUR))//把文件指针指向前一个NALU的末尾
	{
		free(Buf);
		printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
	}

	// Here the Start code, the complete NALU, and the next start code is in the Buf.
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	// 不管有没有再次读到 头 ，其主要 关心的还是 nalu->len
	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	free(Buf);

	return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
}
//输出NALU长度和TYPE
void dump(NALU_t *n)
{
	if (!n)return;
	//printf("a new nal:");
	printf(" len: %d  ", n->len);
	printf("nal_unit_type: %x\n", n->nal_unit_type);
}

int rtp_send_file(int sockfd,struct sockaddr *addr)
{
	printf("into the rtp_send_file...\n");
	int rtp_number;
	OpenBitstreamFile("./doc/test.h264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	NALU_t *n;
	char* nalu_payload;
	char sendbuf[1500];

	unsigned short seq_num =0;
	int	bytes=0;

	float framerate=15;
	unsigned int timestamp_increse=0,ts_current=0;
	timestamp_increse=(unsigned int)(90000.0 / framerate); //+0.5);

	sleep(3);
	n = AllocNALU(80000000);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针

	sleep(3);
	printf("before the GetAnnexbNALU\n");
	while(!feof(bits))
	{
		GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		//dump(n);//输出NALU长度和TYPE

		memset(sendbuf,0,1500);//清空sendbuf；此时会将上次的时间戳清空，因此需要ts_current来保存上次的时间戳值
		//rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给rtp_hdr，以后对rtp_hdr的写入操作将直接写入sendbuf。
		rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0];
		//设置RTP HEADER，
		rtp_hdr->payload     = H264;  //负载类型号，
		rtp_hdr->version     = 2;  //版本号，此版本固定为2
		rtp_hdr->marker    = 0;   //标志位，由具体协议规定其值。
		rtp_hdr->ssrc        = htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一

		//	当一个NALU小于1400字节的时候，采用一个单RTP包发送
		if(n->len<=1400)
		{
			//设置rtp M 位；
			rtp_hdr->marker=1;
			rtp_hdr->seq_no     = htons(seq_num ++); //序列号，每发送一个RTP包增1
			//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
			nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
			nalu_hdr->F=n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendbuf[13];//同理将sendbuf[13]赋给nalu_payload
			memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

			ts_current=ts_current+timestamp_increse;
			rtp_hdr->timestamp=htonl(ts_current);
			bytes=n->len + 12 ;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节
			//	send( socket1, sendbuf, bytes, 0 );//发送rtp包
			rtp_number = sendto(sockfd,sendbuf,bytes,0,addr,16);
			if(-1==rtp_number)
			{
				perror("len < 1400 rtp sendto error");
			}
			else
			{
				printf("len < 1400 rtp_number = %d\n",rtp_number);
			}
			//	Sleep(100);

		}

		else if(n->len>1400)
		{
			//得到该nalu需要用多少长度为1400字节的RTP包来发送
			int k=0,l=0;
			k=n->len/1400;//需要k个1400字节的RTP包
			l=n->len%1400;//最后一个RTP包的需要装载的字节数
			int t=0;//用于指示当前发送的是第几个分片RTP包
			ts_current=ts_current+timestamp_increse;
			rtp_hdr->timestamp=htonl(ts_current);
			while(t<=k)
			{
				rtp_hdr->seq_no = htons(seq_num ++); //序列号，每发送一个RTP包增1
				if(!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
				{
					//设置rtp M 位；
					rtp_hdr->marker=0;
					//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=1;
					fu_hdr->TYPE=n->nal_unit_type;


					nalu_payload=&sendbuf[14];//同理将sendbuf[14]赋给nalu_payload
					memcpy(nalu_payload,n->buf+1,1400);//去掉NALU头

					bytes=1400+14;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
					//send( socket1, sendbuf, bytes, 0 );//发送rtp包
					rtp_number = sendto(sockfd,sendbuf,bytes,0,addr,16);
					if(-1==rtp_number)
					{
						perror("k<=t rtp sendto error");
					}
					else
					{
						printf("k<=t rtp_number = %d\n",rtp_number);
					}
					t++;

				}
				//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
				else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
				{

					//设置rtp M 位；当前传输的是最后一个分片时该位置1
					rtp_hdr->marker=1;
					//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->TYPE=n->nal_unit_type;
					fu_hdr->E=1;

					nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
					memcpy(nalu_payload,n->buf+t*1400+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
					bytes=l-1+14;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
					//send( socket1, sendbuf, bytes, 0 );//发送rtp包
					rtp_number = sendto(sockfd,sendbuf,bytes,0,addr,16);
					if(-1==rtp_number)
					{
						perror("k=t rtp sendto error");
					}
					else
					{
						printf("k=t rtp_number = %d\n",rtp_number);
					}
					t++;
					//	Sleep(100);
				}
				else if(t<k&&0!=t)
				{
					//设置rtp M 位；
					rtp_hdr->marker=0;
					//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					//fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->E=0;
					fu_hdr->TYPE=n->nal_unit_type;

					nalu_payload=&sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
					memcpy(nalu_payload,n->buf+t*1400+1,1400);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
					bytes=1400+14;						//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节
					//send( socket1, sendbuf, bytes, 0 );//发送rtp包
					rtp_number = sendto(sockfd,sendbuf,bytes,0,addr,16);
					if(-1==rtp_number)
					{
						perror("k!=t 0 rtp sendto error");
					}
					else
					{
						printf("k!=t 0 rtp_number = %d\n",rtp_number);
					}
					t++;
				}
			}
		}//>1400
		sleep(1);
	}//while
	FreeNALU(n);
	return 0;
}

static int FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1
	else return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1
	else return 1;
}


static char const* allowedCommandNames
= "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE";

void handleCmd_OPTIONS(char *fResponseBuffer)
{
	// bzero(fResponseBuffer,sizeof (fResponseBuffer));
	snprintf((char*)fResponseBuffer, 256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 1\r\n"
			"Public: %s\r\n\r\n",
			allowedCommandNames);
}

void handleCmd_DESCRIBE(char *fResponseBuffer)
{
	//bzero(fResponseBuffer,sizeof (fResponseBuffer));
	snprintf((char*)fResponseBuffer, 256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 2\r\n"
			"Content-Base: 127.0.0.1\r\n"
			"Content-Type: application/sdp\r\n"
			"m=video 49170 RTP/AVP 98\r\n"
			"a=rtpmap:98 H264/90000\r\n"
			"a=fmtp:98 profile-level-id=42A01E;"
			"sprop-parameter-sets=Z0IACpZTBYmI,aMljiA==\r\n\r\n");
}
void handleCmd_SETUP(char *fResponseBuffer)
{
	//bzero(fResponseBuffer,sizeof (fResponseBuffer));
	snprintf((char*)fResponseBuffer,256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 3\r\n"
			"Transport: RTP/AVP;unicast;port=8980-8981\r\n"
			"Session: 1\r\n\r\n");
}
void handleCmd_PLAY(char *fResponseBuffer)
{
	//bzero(fResponseBuffer,sizeof (fResponseBuffer));
	snprintf((char*)fResponseBuffer,256,
			"RTSP/1.0 200 OK\r\n"
			"CSeq: 5\r\n"
			"Range: npt=0.000-410.134\r\n"
			"User-Agent: yuanzhenhai\r\n\r\n");
}

void handleCmd_bad(char *fResponseBuffer)
{
	//bzero(fResponseBuffer,sizeof (fResponseBuffer));
	//snprintf((char*)fResponseBuffer, 256, "RTSP/1.0 400 Bad Request\r\nAllow: 8\r\n\r\n", allowedCommandNames);
}


void rtsp_yuan(int sockfd,struct sockaddr *addr,char *fResponseBuffer,char *cmdName,int addrlen)
{
	int send_number;
	// if (strcmp(cmdName, "OPTIONS") == 0)
	if(memcmp(cmdName,"OPTIONS",sizeof("OPTIONS")-1)==0)
	{
		printf("option!\n");
		handleCmd_OPTIONS(fResponseBuffer);
		send_number = sendto(sockfd,fResponseBuffer,strlen(fResponseBuffer),0,addr,addrlen);
		if(-1 == send_number)
		{
			perror("option sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("option sendto number = %d\n",send_number);
			printf("option sendto successful!\n");
		}

	}
	// else if (strcmp(cmdName, "DESCRIBE") == 0)
	else if(memcmp(cmdName,"DESCRIBE",sizeof("DESCRIBE")-1)==0)
	{
		printf("describe!\n");
		handleCmd_DESCRIBE(fResponseBuffer);
		send_number = sendto(sockfd,fResponseBuffer,strlen(fResponseBuffer),0,addr,addrlen);
		if(-1 == send_number)
		{
			perror("describe sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("describe sendto number = %d\n",send_number);
			printf("descrebe sendto successful!\n");
		}
	}
	//else if (strcmp(cmdName, "SETUP") == 0)
	else if(memcmp(cmdName,"SETUP",sizeof("SETUP")-1)==0)
	{
		printf("setup!\n");
		handleCmd_SETUP(fResponseBuffer);
		// printf("addrlen=%s",(char*)addrlen);
		//printf("addr.sa_data=%s\n",*addr->sa_data);
		send_number = sendto(sockfd,fResponseBuffer,strlen(fResponseBuffer),0,addr,addrlen);
		if(send_number == -1)
		{
			perror("setup sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("setup sendto number = %d\n",send_number);
			printf("setup sendto successful!\n");
		}

	}
	//else if ( (ii=strcmp(cmdName, "PLAY")) == 0)
	else if(memcmp(cmdName,"PLAY",sizeof("PLAY")-1)==0)
	{
		printf("play!\n");
		handleCmd_PLAY(fResponseBuffer);
		send_number = sendto(sockfd,fResponseBuffer,strlen(fResponseBuffer),0,addr,addrlen);
		if(-1 == send_number)
		{
			perror("play sendto error");
		}
		else
		{
			int rtp_flag = rtp_send_file(sockfd,addr);
			printf("rtp_flag = %d",rtp_flag);
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("play sendto number = %d\n",send_number);
			printf("play sendto successful!\n");
		}

	}
	else
	{
		printf("bad request!\n");
		handleCmd_bad(fResponseBuffer);
		send_number = sendto(sockfd,fResponseBuffer,strlen(fResponseBuffer),0,addr,addrlen);
		if(-1 == send_number)
		{
			perror("bad reqest sendto error");
		}
		else
		{
			printf("fResponseBuffer:%s\n",fResponseBuffer);
			printf("bad request sendto number = %d\n",send_number);
			printf("bad reqrest successful!\n");
		}

	}
	printf("rtsp_yuan() success!\n");
}



char *get_cmd_name(char *request)
{
	char *temp_str = malloc(sizeof (char));
	temp_str = request;
	int count;
	char temp,temp11=' ';
	char *temp1=(char*)malloc(sizeof(char));
	for(count=0;count<(sizeof(temp_str));count++)
	{
		temp = temp_str[count];
		if(temp == temp11)
			break;
	}
	memcpy(temp1,temp_str,count);
	printf("temp_str is %s\n",temp1);
	return temp_str;
}

#if 1

#if 0
static void set_keepalive_params(int sockfd, int timeout, int count, int intvl)
{
	int keepalive_time = timeout;
	int keepalive_probes = count;
	int keepalive_intvl = intvl;

	/*对一个连接进行有效性探测之前运行的最大非活跃时间间隔，默认值为 14400（即 2 个小时）*/
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(int)) < 0) {
		printf("TCP_KEEPIDLE failed");
		return;
	}

	/*关闭一个非活跃连接之前进行探测的最大次数，默认为 8 次 */
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(int)) < 0) {
		printf("TCP_KEEPCNT failed");
		return;
	}

	/*两个探测的时间间隔，默认值为 150 即 75 秒,失败时候调用*/
	if(setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(int)) < 0) {
		printf("TCP_KEEPINTVL failed");
		return;
	}

	return;

}


/*********************************************************************************
* Description:设置套接字的超时时间
********************************************************************************/
int mid_socket_set_active(int sockfd, int timeout, int count, int intvl)
{
	int optval;
	socklen_t optlen = sizeof(optval);

	/* check the status for the keepalive option */
	if(getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
		printf("getsockopt SO_KEEPALIVE failed");
		return -1;
	}

	printf("SO_KEEPALIVE is %s\n", optval ? "ON" : "OFF");

	/* set the option active */
	optval = 1;
	optlen = sizeof(optval);

	if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
		printf("setsockopt SO_KEEPALIVE failed，reason: %m\n");
		return -1;
	}

	printf("SO_KEEPALIVE on socket\n");

	/* check the status again */
	if(getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
		printf("getsockopt SO_KEEPALIVE again failed");
		return -1;
	}

	set_keepalive_params(sockfd, timeout, count, intvl);
	printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));
	return 0;
}


/*********************************************************************************************
 * Description:对新连入的套接字进行属性设置
 *********************************************************************************************/
int setSocketAttrClient(const int fd)
{
	int activetime = 7200;
	mid_socket_set_active(fd, activetime, 3, 30);
	if((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&nSize, sizeof(nSize))) == -1) {
		printf("[%s]:setsockopt=%s line:%d\n", __func__, strerror(errno), __LINE__);
	}

	int nSize = 0;
	int nLen = sizeof(nLen);
	int result = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&nSize , (socklen_t *)&nLen);
	if(result) {
		printf("[%s]:setsockopt=%s line:%d\n", __func__, strerror(errno), __LINE__);
	}

	nSize = 1;

	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&nSize , sizeof(nSize))) {
		printf("[%s]:setsockopt=%s line:%d\n", __func__, strerror(errno), __LINE__);
	}


	return 0;
}
#endif


/****************************************************************************************
* Description:线程函数，处理具体的rtsp命令
****************************************************************************************/
//void dealWithRtspCmd(void *inParam)
void RTSPProcess(void *inParam)
{
	int ret = 0;
	RTSP_ATTR *p_rtspAttr = (RTSP_ATTR*)inParam; 
	RTSP_ATTR hisRtspAttr;
	RTSP_MSG_ATTR rtspMsgAttr;
	char *frecv_buffer = (char*)malloc(RX_BUF_LEN);
	char fsend_buffer[256] = {0};// = (char*)malloc(sizeof(char));
	
	memset(&hisRtspAttr, 0, sizeof(hisRtspAttr));
	trace(DEBUG, "[%s]:create new pthread------------------------------------------ line:%d\n", __func__, __LINE__);
	while(1)
	{
#if 1
		int len = sizeof(p_rtspAttr->addr);
		memset(frecv_buffer, 0, strlen(frecv_buffer));
		//trace(DEBUG, "[%s]:----------------------line:%d\n", __func__, __LINE__);
		//frecv_buffer = sock_recv((p_rtspAttr->fd),(struct sockaddr *)&(p_rtspAttr->addr),(int *)&len);
		ret = read_data((sint32*)&(p_rtspAttr->fd), frecv_buffer, RX_BUF_LEN);
		if(0>=ret){
			continue;
		}
		trace(DEBUG, "[%s]:----------------------line:%d\n", __func__, __LINE__);
		//trace(DEBUG, "\e[0m[%s]:recv[%d]\e[0m:%s line:%d\n", __func__, strlen(frecv_buffer), frecv_buffer, __LINE__);
		trace(DEBUG, "\e[33m[%s]:recv[%d]:begin rtsp cmd! \e[0m line:%d\n", __func__, strlen(frecv_buffer), __LINE__);
		//rtsp_yuan((p_rtspAttr->fd),(struct sockaddr *)&(p_rtspAttr->addr),fsend_buffer,frecv_buffer,16);
		parseVLCMsg(frecv_buffer, &rtspMsgAttr);
		dealWirhRtspCmd(p_rtspAttr, &rtspMsgAttr);
		trace(DEBUG, "\e[33m[%s]:recv[%d]:end rtsp cmd!---- \e[0m line:%d\n", __func__, strlen(frecv_buffer), __LINE__);
		usleep(1000*1000);
#endif
	}
	
	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return;
}

/*********************************************************************************************************************
* Description:测试例子
*
**********************************************************************************************************************/
int testDemoRtspServer()
{
	int ret = 0;
	pthread_t pthreadID;
	struct sockaddr_in clientAddr, hisClientAddr;
	NETWORK_ATTR serverNetAttr = {"192.168.5.192", 8800, 0};
	RTSP_ATTR rtspClientAttr;

	initPrintAndPthread(); /*初始化打印级别和异常信号处理*/
	memset(&rtspClientAttr, 0, sizeof(rtspClientAttr));
	memset(&hisClientAddr, 0, sizeof(hisClientAddr));
	serverNetAttr.fd = createSocket(AF_INET, SOCK_STREAM, 0);
	//bindSocket((serverNetAttr.fd), AF_INET, (serverNetAttr.ip));
	bindSocket2((serverNetAttr.fd), AF_INET, (serverNetAttr.ip), (serverNetAttr.port));
	if(listenSocket((serverNetAttr.fd), 3) < 0){
		trace(ERROR, "[%s]:listen timeout! line:%d\n", __func__, __LINE__);
		return 0;
	}

	while(1)
	{
		sleep(5);
		trace(DEBUG,"[%s]:server ip=%s port=%d fd=%d line:%d\n", __func__, (serverNetAttr.ip), (serverNetAttr.port), (serverNetAttr.fd), __LINE__);
		memset(&clientAddr, 0, sizeof(clientAddr));
		ret = 0;
		ret = acceptSocket2((serverNetAttr.fd), &clientAddr);
		if(0 >= ret){
			continue;
			trace(ERROR, "[%s][Error]:accept failure re=%d line:%d\n", __func__, ret, __LINE__);
			sleep(500);
		}
		/*如果地址相同不在进行处理*/
		if(0 == strncmp((char*)&clientAddr, (char*)&hisClientAddr, sizeof(clientAddr))){
			trace(DEBUG, "[%s]:same client! line:%d\n", __func__, __LINE__);
			continue;
		}
		memcpy(&hisClientAddr, &clientAddr, sizeof(hisClientAddr));
		/*处理新的连接请求*/
		trace(DEBUG, "[%s]:client ip=%s port=%d fd=%d line:%d\n", __func__, inet_ntoa(clientAddr.sin_addr), (int)ntohs(clientAddr.sin_port), ret, __LINE__);
		rtspClientAttr.fd = ret;
		memcpy(&(rtspClientAttr.addr), &(clientAddr), sizeof(rtspClientAttr.addr));
		pthread_create(&pthreadID, NULL, (void *)RTSPProcess, (void *)&rtspClientAttr);
#if 0
		if(0 == strncmp((char*)&clientAddr, (char*)&hisClientAddr, sizeof(clientAddr))){
			printf("[%s][Debug]:same client! line:%d\n", __func__, __LINE__);
			continue;
		}
		memset(&hisClientAddr, 0, sizeof(hisClientAddr));
		memcpy(&hisClientAddr, &clientAddr, sizeof(clientAddr));
		//close(serverNetAttr.fd);
		printf("[%s][Debug]:\e[32maccept ok! line:%d\e[0m\n", __func__, __LINE__);

		/*创建一个线程来处理具体的RTSP命令*/
		//ret = 0;
		result = pthread_create(&serverThid2, NULL, (void *)dealWithRtspCmd, (void *)clientAddr);
		//ret = pthread_create(&rtsp_threadid[nPos], NULL, (void *)RtspVlcContact, (void *)rtspthread);
#endif
#if 0
		int len = sizeof(clientAddr);
		char *frecv_buffer = (char*)malloc(sizeof(char));
		char fsend_buffer[256] = {0};// = (char*)malloc(sizeof(char));
		memset(frecv_buffer, 0, strlen(frecv_buffer));
		frecv_buffer = sock_recv(ret,(struct sockaddr *)&clientAddr,(int *)&len);
		printf("[%s]:recv[%d]:%s line:%d\n", __func__, strlen(frecv_buffer), frecv_buffer, __LINE__);
		rtsp_yuan((ret),(struct sockaddr *)&clientAddr,fsend_buffer,frecv_buffer,16);
#endif
		//setSocketAttrClient(serverNetAttr.fd);
	}
	
	return 0;
}

#endif

/******************************************************************************
 * Description:测试程序的入口
 * 
 * *******************************************************************************/
int main()
//int mainRtsp()
{
	testDemoRtspServer();
#if 0
	int server_port = 8800;
	//int sockfd,sockfd1;
	int sockfd;
	struct sockaddr_in addr;
	struct sockaddr_in addr_client;
	int len;

	//char *cmdname = (char*)malloc(sizeof(char));
	char *frecv_buffer = (char*)malloc(sizeof(char));
	char fsend_buffer[256];// = (char*)malloc(sizeof(char));

	// sockfd1=sock_init(sockfd,addr_server,server_port);
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

	if(sockfd<0)
	{
		fprintf(stderr,"Socket Error:%s\n",strerror(errno));
		exit(1);
	}
	//printf("sockfd is %d\n",sockfd);
	bzero(&addr,sizeof(struct sockaddr_in));

	addr.sin_family=AF_INET;
	//addr.sin_addr.s_addr=htonl(INADDR_ANY);
	char serverIP[64] = {"192.168.5.192"};
	//memcpy((addr.sin_addr.s_addr), serverIP, strlen(serverIP) );
	inet_pton(AF_INET, serverIP, &(addr.sin_addr));
	addr.sin_port=htons(server_port);

	if(bind(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))<0)
	{
		fprintf(stderr,"Bind Error:%s\n",strerror(errno));
		exit(1);
	}
	printf("init successfel!!\nport is %dsockfd is %d\n",server_port,sockfd);
	// printf("init sockfd is %d\n",sockfd);
	while (1)
	{
		printf("[%s]:block for sockrecv .......... line:%d\n", __func__, __LINE__);
		frecv_buffer = sock_recv(sockfd,(struct sockaddr *)&addr_client,(int *)&len);
		printf("[%s]:recvbuffer is %s line:%d\n", __func__, frecv_buffer, __LINE__);
		char IPdotdec[20]; //存放点分十进制IP地址
		struct in_addr s = addr_client.sin_addr;
		inet_ntop(AF_INET, (void *)&s, IPdotdec, 16);
		printf("[%s]:addr_client.data=%s line:%d\n", __func__, IPdotdec, __LINE__);
		printf("[%s]:len=%d line:%d\n", __func__, len, __LINE__);
		printf("[%s]:begin rtsp_yuan() .......... line:%d\n", __func__, __LINE__);

		rtsp_yuan(sockfd,(struct sockaddr *)&addr_client,fsend_buffer,frecv_buffer,16);
		// sleep(3);
	}
	return 0;
#endif
}
