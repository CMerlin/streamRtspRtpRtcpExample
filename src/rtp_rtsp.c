// NALDecoder.cpp : Defines the entry point for the console application.
//


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
#include <unistd.h>    //close()


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
static int FindStartCode2 (unsigned char *Buf);//���ҿ�ʼ�ַ�0x000001
static int FindStartCode3 (unsigned char *Buf);//���ҿ�ʼ�ַ�0x00000001
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

char* sock_recv(int sockfd,struct sockaddr *addr_client,int *addrlen)
{
	//int *tem_len = &addrlen;
	socklen_t len;
	printf("sock_recv sockfd is %d\n",sockfd);
	char *recv_buffer = malloc (sizeof (char));
	//printf("sock_recv sockfd is88888888888888 %d\n",sockfd);
	int n;
	n=recvfrom(sockfd,recv_buffer,256,0,addr_client,&len);
	//printf("recv number is %d\n",n);
	if(0)
	{
		printf("recvfrom error!\n");
		exit (1);
	}
	if(-1==n)
	{
		perror("recv error:");
	}
	else
	{
		addrlen=(int *)len;
		printf("sock recv success!!\n");
		/**   char IPdotdec[20]; //��ŵ��ʮ����IP��ַ
		  struct in_addr s =
		  inet_ntop(AF_INET, (void *)&s, IPdotdec, 16);
		  printf("addr_client.data=%s\n",IPdotdec);
		  */ printf("addr_len=%d\n", *addrlen);
	}
	return recv_buffer;
}

//ΪNALU_t�ṹ������ڴ�ռ�
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
//�ͷ�
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
//�����������Ϊһ��NAL�ṹ�壬��Ҫ����Ϊ�õ�һ��������NALU��������NALU_t��buf�У���ȡ���ĳ��ȣ����F,IDC,TYPEλ��
//���ҷ���������ʼ�ַ�֮�������ֽ�������������ǰ׺��NALU�ĳ���
int GetAnnexbNALU (NALU_t *nalu)
{
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf;

	if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
		printf ("GetAnnexbNALU: Could not allocate Buf memory\n");

	nalu->startcodeprefix_len=3;//��ʼ���������еĿ�ʼ�ַ�Ϊ3���ֽ�

	if (3 != fread (Buf, 1, 3, bits))//�������ж�3���ֽ�
	{
		free(Buf);
		return 0;
	}
	info2 = FindStartCode2 (Buf);//�ж��Ƿ�Ϊ0x000001
	if(info2 != 1)
	{
		//������ǣ��ٶ�һ���ֽ�
		if(1 != fread(Buf+3, 1, 1, bits))//��һ���ֽ�
		{
			free(Buf);
			return 0;
		}
		info3 = FindStartCode3 (Buf);//�ж��Ƿ�Ϊ0x00000001
		if (info3 != 1)//������ǣ�����-1
		{
			free(Buf);
			return -1;
		}
		else
		{
			//�����0x00000001,�õ���ʼǰ׺Ϊ4���ֽ�
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}

	else
	{
		//�����0x000001,�õ���ʼǰ׺Ϊ3���ֽ�
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}
	//������һ����ʼ�ַ��ı�־λ
	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

	while (!StartCodeFound)
	{
		if (feof (bits))//�ж��Ƿ����ļ�β
		{
			nalu->len = (pos-1)-nalu->startcodeprefix_len;
			memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			free(Buf);
			return pos-1;
		}
		Buf[pos++] = fgetc (bits);//��һ���ֽڵ�BUF��
		info3 = FindStartCode3(&Buf[pos-4]);//�ж��Ƿ�Ϊ0x00000001
		if(info3 != 1)
			info2 = FindStartCode2(&Buf[pos-3]);//�ж��Ƿ�Ϊ0x000001
		StartCodeFound = (info2 == 1 || info3 == 1);
	}



	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;

	if (0 != fseek (bits, rewind, SEEK_CUR))//���ļ�ָ��ָ��ǰһ��NALU��ĩβ
	{
		free(Buf);
		printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
	}

	// Here the Start code, the complete NALU, and the next start code is in the Buf.
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	// ������û���ٴζ��� ͷ ������Ҫ ���ĵĻ��� nalu->len
	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//����һ������NALU����������ʼǰ׺0x000001��0x00000001
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	free(Buf);

	return (pos+rewind);//����������ʼ�ַ�֮�������ֽ�������������ǰ׺��NALU�ĳ���
}
//���NALU���Ⱥ�TYPE
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
	OpenBitstreamFile("./test.264");//��264�ļ��������ļ�ָ�븳��bits,�ڴ��޸��ļ���ʵ�ִ򿪱��264�ļ���
	NALU_t *n;
	char* nalu_payload;
	char sendbuf[1500];

	unsigned short seq_num =0;
	int	bytes=0;

	float framerate=15;
	unsigned int timestamp_increse=0,ts_current=0;
	timestamp_increse=(unsigned int)(90000.0 / framerate); //+0.5);

	sleep(3);
	n = AllocNALU(80000000);//Ϊ�ṹ��nalu_t�����Աbuf����ռ䡣����ֵΪָ��nalu_t�洢�ռ��ָ��

	sleep(3);
	printf("before the GetAnnexbNALU\n");
	while(!feof(bits))
	{
		GetAnnexbNALU(n);//ÿִ��һ�Σ��ļ���ָ��ָ�򱾴��ҵ���NALU��ĩβ����һ��λ�ü�Ϊ�¸�NALU����ʼ��0x000001
		//dump(n);//���NALU���Ⱥ�TYPE

		memset(sendbuf,0,1500);//���sendbuf����ʱ�Ὣ�ϴε�ʱ�����գ������Ҫts_current�������ϴε�ʱ���ֵ
		//rtp�̶���ͷ��Ϊ12�ֽ�,�þ佫sendbuf[0]�ĵ�ַ����rtp_hdr���Ժ��rtp_hdr��д�������ֱ��д��sendbuf��
		rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0];
		//����RTP HEADER��
		rtp_hdr->payload     = H264;  //�������ͺţ�
		rtp_hdr->version     = 2;  //�汾�ţ��˰汾�̶�Ϊ2
		rtp_hdr->marker    = 0;   //��־λ���ɾ���Э��涨��ֵ��
		rtp_hdr->ssrc        = htonl(10);    //���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ

		//	��һ��NALUС��1400�ֽڵ�ʱ�򣬲���һ����RTP������
		if(n->len<=1400)
		{
			//����rtp M λ��
			rtp_hdr->marker=1;
			rtp_hdr->seq_no     = htons(seq_num ++); //���кţ�ÿ����һ��RTP����1
			//����NALU HEADER,�������HEADER����sendbuf[12]
			nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�
			nalu_hdr->F=n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendbuf[13];//ͬ��sendbuf[13]����nalu_payload
			memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����

			ts_current=ts_current+timestamp_increse;
			rtp_hdr->timestamp=htonl(ts_current);
			bytes=n->len + 12 ;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�
			//	send( socket1, sendbuf, bytes, 0 );//����rtp��
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
			//�õ���nalu��Ҫ�ö��ٳ���Ϊ1400�ֽڵ�RTP��������
			int k=0,l=0;
			k=n->len/1400;//��Ҫk��1400�ֽڵ�RTP��
			l=n->len%1400;//���һ��RTP������Ҫװ�ص��ֽ���
			int t=0;//����ָʾ��ǰ���͵��ǵڼ�����ƬRTP��
			ts_current=ts_current+timestamp_increse;
			rtp_hdr->timestamp=htonl(ts_current);
			while(t<=k)
			{
				rtp_hdr->seq_no = htons(seq_num ++); //���кţ�ÿ����һ��RTP����1
				if(!t)//����һ����Ҫ��Ƭ��NALU�ĵ�һ����Ƭ����FU HEADER��Sλ
				{
					//����rtp M λ��
					rtp_hdr->marker=0;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=1;
					fu_hdr->TYPE=n->nal_unit_type;


					nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]����nalu_payload
					memcpy(nalu_payload,n->buf+1,1400);//ȥ��NALUͷ

					bytes=1400+14;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥ��ʼǰ׺��NALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�
					//send( socket1, sendbuf, bytes, 0 );//����rtp��
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
				//����һ����Ҫ��Ƭ��NALU�ķǵ�һ����Ƭ������FU HEADER��Sλ������÷�Ƭ�Ǹ�NALU�����һ����Ƭ����FU HEADER��Eλ
				else if(k==t)//���͵������һ����Ƭ��ע�����һ����Ƭ�ĳ��ȿ��ܳ���1400�ֽڣ���l>1386ʱ����
				{

					//����rtp M λ����ǰ����������һ����Ƭʱ��λ��1
					rtp_hdr->marker=1;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->TYPE=n->nal_unit_type;
					fu_hdr->E=1;

					nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
					memcpy(nalu_payload,n->buf+t*1400+1,l-1);//��nalu���ʣ���l-1(ȥ����һ���ֽڵ�NALUͷ)�ֽ�����д��sendbuf[14]��ʼ���ַ�����
					bytes=l-1+14;		//���sendbuf�ĳ���,Ϊʣ��nalu�ĳ���l-1����rtp_header��FU_INDICATOR,FU_HEADER������ͷ��14�ֽ�
					//send( socket1, sendbuf, bytes, 0 );//����rtp��
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
					//����rtp M λ��
					rtp_hdr->marker=0;
					//����FU INDICATOR,�������HEADER����sendbuf[12]
					fu_ind =(FU_INDICATOR*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
					fu_ind->F=n->forbidden_bit;
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//����FU HEADER,�������HEADER����sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendbuf[13];
					//fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->E=0;
					fu_hdr->TYPE=n->nal_unit_type;

					nalu_payload=&sendbuf[14];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
					memcpy(nalu_payload,n->buf+t*1400+1,1400);//ȥ����ʼǰ׺��naluʣ������д��sendbuf[14]��ʼ���ַ�����
					bytes=1400+14;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥԭNALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�
					//send( socket1, sendbuf, bytes, 0 );//����rtp��
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
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //�ж��Ƿ�Ϊ0x000001,����Ƿ���1
	else return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//�ж��Ƿ�Ϊ0x00000001,����Ƿ���1
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

/******************************************************************************
 * Description:���Գ�������
 * 
 * *******************************************************************************/
//int main()
int mainRtsp()
{
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
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
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
		printf("block for sockrecv ..........\n");
		frecv_buffer = sock_recv(sockfd,(struct sockaddr *)&addr_client,(int *)&len);
		printf("recvbuffer is %s\n",frecv_buffer);
		char IPdotdec[20]; //��ŵ��ʮ����IP��ַ
		struct in_addr s = addr_client.sin_addr;
		inet_ntop(AF_INET, (void *)&s, IPdotdec, 16);
		printf("addr_client.data=%s\n",IPdotdec);
		printf("len=%d\n",len);
		printf("begin rtsp_yuan() ..........\n");

		rtsp_yuan(sockfd,(struct sockaddr *)&addr_client,fsend_buffer,frecv_buffer,16);
		// sleep(3);
	}
	return 0;
}

