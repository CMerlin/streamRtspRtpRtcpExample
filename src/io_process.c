/****************************************************************
 * Copyright(C), 2016, Hanbang Tech.Co.,Ltd
 * File Name:slave_http
 * Author:merlin
 * Version:v1
 * Data:20160613
 * Description:����һ��HTTPЭ��Ŀͻ��˳���
 * History:
 * *************************************************************/
#include "common.h"

/******************************************************************************
 * Description����ȡ�����ļ�����ȡhttp�������˿ں�
 * Input path�������ļ�·��
 * Output port���˿ں�
 * Return��0-�ɹ� -1-ʧ��
 * *****************************************************************************/
sint32 get_config_info(void *path, sint32 *port)
{
	sint32 ret = 0;
	uint8 buffer[1024] = {0};

	ret = open(path, O_RDONLY);
	if(ret < 0)
	{
		trace(ERROR, "[%s]:open=%s LINE:%d\n", __func__,  strerror(errno),__LINE__);
		return -1;
	}

	while(1)
	{
		ret = read(ret, buffer, sizeof(buffer));
		if(ret <= 0)
			break;
		*port = atoi((char*)buffer);
		trace(TRACE, "[%s]:buffer=%s port=%d LINE:%d\n", __func__, buffer, (*port), __LINE__);	
	}

	return 0;
}

/*******************************************************
 * Description�����Ŀ���ļ��Ƿ�ɶ���д
 * Input file����Ҫ�����ļ�
 * Return��0-�ɶ� 1-��д -1-��ʱ�����
 * *****************************************************/
sint32 is_read_write(const sint32 *file) 
{
	sint32 ret = 0;
	struct pollfd fds[1];
	memset(fds, '\0', sizeof(struct pollfd) * 1);
	fds[0].fd = (*file);
	fds[0].events = (POLLIN | POLLOUT);
	ret = poll(fds, 1, 3000); //1000����ĳ�ʱʱ��
	if(ret <= 0) //��ʱ�����
		return -1;
	if(fds[0].revents & POLLIN) //�ɶ�
		return 0;
	else if(fds[0].revents & POLLOUT) //��д
		return 1;

	return -1;
}

#if 0
/*******************************************************
 * Description����TCP�������˽���һ�����ӣ��������ķ�ʽ���������ǳ�����Ҳ�����Ƕ�����
 * Input serer������������Ҫ�õ���Ϣ
 * Output server�����Ӷ��õ�����Ϣ
 * Return��0-�ɹ� <0-�������
 * *****************************************************/
sint32 connect_server_nonblock(NETWORK_ATTR_S *server)
{
	if(server->connect == 1)
		return 0;
#if 0
	/*������������ӵ�ʱ����*/
	long sint32 now = time(NULL);
	long sint32 sint32erval = now - (history.btime);
	//server->btime = time(NULL);
	trace(DEBUG, "[%s]:sint32erval=%ld now=%ld server->btime=%ld LINE:%d\n", __func__, sint32erval, now, history.btime, __LINE__);
	//if(sint32erval < 5)
	if(sint32erval < 2)
		return 0;
#endif
	sint32 ret = 0;
	/*�����׽���*/
	if(server->fd > 0)
		close(server->fd);
	server->fd = 0;
	server->fd = createSocket(AF_INET, SOCK_STREAM, 0);
	if(server->fd < 0){
		trace(ERROR, "[%s]:createSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	/*����������*/
	ret = connectSocket(&(server->fd), AF_INET, server->ip, server->port);
	if(ret < 0){
		trace(ERROR, "[%s]:connectSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	/*���óɹ��������ӵı�־*/
	server->connect = 1;
	server->btime = time(NULL);

	return 0;
}


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

	/*���IP�ı���뽨���µ�����*/
	memset(&history, 0, sizeof(history));
	isapi_get_connect_status(&history);
	trace(DEBUG, "[%s]:history connect info fd=%d ip=%s port=%d connect=%d type=%d LINE:%d\n", __func__, history.fd, history.ip, history.port, history.connect, history.type, __LINE__);
	if(0 != strncmp(history.ip, server->ip, strlen(history.ip))) //IP�Ƿ�ı�
	{
		history.connect = 0;
		close(history.fd);
	}

	/*����Ƕ����ӣ������µ�����*/
	if(server->type != KEEP_ALIVE)
	{
		history.connect = 0;
	}

	if(history.connect == 1) //����״̬
	{
		(*server) = history;
		return 0;
	}

	if(server->fd > 0)
	{
		close(server->fd);
	}
	server->fd = 0;
	
	/*�����׽���*/
	server->btime = time(NULL);
	server->fd = createSocket(AF_INET, SOCK_STREAM, 0);
	if(server->fd < 0)
	{
		trace(ERROR, "[%s]:createSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return CREATE_SOCKET;
	}
	/*����������*/
	trace(TRACE, "[%s]:try connect to server->ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
	ret = connectSocket(&(server->fd), AF_INET, server->ip, server->port);
	if(ret < 0)
	{
		trace(ERROR, "[%s]:connectSocket excuate faild! ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
		return CONNECT_FAILED;
	}
	/*���óɹ��������ӵı�־*/
	server->connect = 1;
	memset(&history, 0, sizeof(history));
	history = (*server);
	isapi_set_connect_status(&history);

	return 0;
}
#endif

/*
 * ��������˵������������
 * �����������Ҫ���͵�����֡
 * ���������
 * ��������ֵ��
 * */
/******************************************************************************
 * Description��������д��Ŀ���ļ���
 * Input file��Ŀ���ļ�
 * Input data����Ҫд���ļ�������
 * Input size�����ݵĴ�С
 * Return��0-�ɹ� -1-ʧ��
 * *****************************************************************************/
sint32 send_data(const sint32 *file, const void *data, const sint32 size)
{
	if((*file < 0) || (size <= 0))
	{
		return -1;
	}

	sint32 ret = 0;

	/*����ļ��Ƿ��д*/
	if(1 != is_read_write(file))
	{
		trace(ERROR, "[%s]:the targer file cannot be written! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	/*��������*/
	tcflush(*file, TCIOFLUSH);
	ret = write(*file, data, size);
	//trace(DEBUG, "[%s]:[write:%d]:data=%s LINE:%d\n", __func__, ret, data, __LINE__);
	if(ret <= 0)
	{
		trace(ERROR, "[%s]:write=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	//tcflush(file, TCIOFLUSH);

	return ret;
}

/******************************************************************************
 * Description����Ŀ���ļ��϶�ȡ����
 * Input file����Ҫ��ȡ���ļ�
 * Input size������ȥ���Ĵ�С
 * Output data����ȡ��������
 * Return��-1:���� !=-1:��ȡ��������֡�ĳ���
 * *****************************************************************************/
sint32 read_data(const sint32 *file, void *data, sint32 size)
{
	if((*file < 0) || (size <= 0))
		return -1;

	sint32 ret = 0;
	
	/*����ļ��Ƿ�ɶ�*/
	if(0 != is_read_write(file)) 
	{
		usleep(1000);
		trace(ERROR, "[%s]:the targer file cannot read! file=%d LINE:%d\n", __func__, (*file), __LINE__);
		return -1;
	}
	/*��ȡ�ļ��е�����*/
	ret = read(*file, data, size);
	trace(DEBUG, "[%s]:[read:%d] LINE:%d\n", __func__, ret, __LINE__);
	if(ret < 0)
	{
		trace(ERROR, "[%s]:read=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	tcflush(*file, TCIOFLUSH);

	return ret;
}

/******************************************************************************
 * Description�����Ŀ������֡
 * Input buffer����Ҫ���м�������֡
 * Input attr������֡����������
 * Return��0-�ɹ� -1-δ��⵽ͷ -2-δ��⵽β -3-δ��⵽������ַ���  -4-����֡�ĳ��Ȳ���
 * *****************************************************************************/
sint32 packet_check(const void *buffer, PACKET_ATTR_S *attr)
{
	switch(attr->attr)
	{
	case PACKET_ATTR_HEADANDTAIL_E: //head and tail
		if(NULL == strstr(buffer, attr->head))
			return PACKET_ATTR_HEAD_E; //δ��⵽ͷ
		if(NULL == strstr(buffer, attr->tail))
			return PACKET_ATTR_TAIL_E; //δ��⵽β
		break;
	case PACKET_ATTR_HEAD_E: //only the head
		if(NULL == strstr(buffer, attr->tail))
			return  PACKET_ATTR_HEAD_E; //δ��⵽ͷ
		break;
	case PACKET_ATTR_SPECIAL_E: //only special string 
		if(NULL == strstr(buffer, attr->special))
			return PACKET_ATTR_SPECIAL_E; //δ����������ַ���
		break;
	case PACKET_ATTR_LEN_E: //fixed length
		if(attr->len > strlen(buffer))
			return PACKET_ATTR_LEN_E; //���Ȳ���
		break;
	default:
		break;
	}
	return 0;
}

#if 0
/******************************************************************************
 * Description����Ŀ���ļ��϶�ȡ����
 * Input file����Ҫ��ȡ���ļ�
 * Input attr�����ݰ���������
 * Input size������ȥ���Ĵ�С
 * Output data����ȡ��������
 * Output��-1:���� !=-1:��ȡ��������֡�ĳ���
 * *****************************************************************************/
sint32 receive_data(const sint32 *file, PACKET_ATTR_S *attr, void *data, sint32 size)
{	
	if((file == NULL) || (data == NULL) || (size < 0))
	{
		trace(ERROR, "[%s]:param wrong! LINE:%d\n", __func__, __LINE__);
		return -1;
	}

	sint32 i = 0, ret = 0, data_len1 = 0, data_len2 = 0;
	for(i = 0; i < 3; i++) //�����������ѭ����ȡ����
	{
		data_len1 = read_data(file, data, size);
		if(data_len1 < 0)
		{
			trace(TRACE, "[%s]:timeout handler! LINE:%d\n", __func__, __LINE__);
			continue;
		}
		break;
	}
	
	/*�������ζ�û�ж���������ʱ����*/
	if(i >= 3)
	{
		trace(ERROR, "[%s]:read timeout! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	
	/*������������֡�������*/
	ret = packet_check(data, attr);
	if(ret >= 0) //�յ�һ������������֡
		return strlen(data);

	sint32 buffer_size = 10240;
	void *buffer = (void*)malloc(buffer_size);
	if(buffer == NULL)
	{
		trace(ERROR, "[%s]:malloc=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	memset(buffer, '\0', buffer_size);
	
	/*�յ�������֡��������Ҫ���зְ�����*/
	data_len1 = strlen(data); //��һ�����ݵĳ���
	trace(DEBUG, "[%s]:deal with subpackage. LINE:%d\n", __func__, __LINE__);
	if(ret == PACKET_ATTR_TAIL_E) //�ְ�����
	{
		ret = read_data(file, buffer, buffer_size);
		if(ret < 0)
		{
			trace(ERROR, "[%s]:Didn't receive data! LINE;%d\n", __func__, __LINE__);
			free(buffer);
			return strlen(data);
		}
	}

	/*�յ�һ���µ����ݰ�*/
	data_len2 = strlen(buffer); //�ڶ������ݵĳ���
	if(NULL != strstr(buffer, attr->head))
	{
		trace(TRACE, "[%s]:receive new package! LINE:%d\n", __func__, __LINE__);
		memcpy(data, buffer, data_len2);
		free(buffer);
		return data_len2;
	}

	/*ƴ�����ݰ�*/
	if((size-1) < (data_len1+data_len2)) //������С��
	{
		trace(ERROR, "[%s]:Cache space is too small! data_len=%d LINE:%d\n", __func__, (data_len1+data_len2),__LINE__);
		memcpy((data+data_len1), buffer, (size-data_len1));
		free(buffer);
		return size;
	}
	else
	{
		memcpy((data + data_len1), buffer, data_len2);
		free(buffer);
		return (data_len1 + data_len2);
	}
	
	return 0;
}

/******************************************************************************
 * Description����http����֡��ȡ��xml���ݶΣ��˷�������Ҫ֪��xml����֡�ĸ��ڵ�����
 * Input src��Դ�ַ���
 * Output dest��Ŀ���ַ���
 * Input dest_size��Ŀ���ַ�������ռ�Ĵ�С
 * Input node_name��xml�ļ���һ���ڵ�Ľڵ����ƣ�ע��˽ڵ���ַ���������Ψһ��
 * Return��0-�ɹ� -1-ʧ��
 * *****************************************************************************/
int getXmlData(const void *src, uint8 *dest, int dest_size, uint8 *node_name)
{
	int size = strlen(src);
	if((src == NULL) || (node_name == NULL) || (size <= 0))
	{
		trace(DEBUG, "[%s]:parameter is null, or less than zero! LINE:%d\n", __func__, __LINE__);
		return -1;
	}

	int len1 = 0, len2 = 0, len3 = 0;
	char *ptr1 = NULL, *ptr2 = NULL, alias[64] = {0};;
	void *buffer = (void*)realloc(NULL, (size+1));  
	memcpy(buffer, src, size);
	
	/*���xml�ļ���������*/
	ptr1 = strstr(buffer, "?xml version="); //����xml�ļ�ͷ
	if(ptr1 == NULL)
	{
		trace(DEBUG, "[%s]:Didn't find the string \"?xml version=\"! LINE:%d\n", __func__, __LINE__);
		free(buffer);
		return -1;
	}
	memset(alias, '\0', strlen(alias));
	sprintf(alias, "<%s", node_name);
	ptr2 = strstr(ptr1, alias); //���ҵ�һ���ڵ��ͷ��
	if(ptr2 == NULL)
	{
		trace(DEBUG, "[%s]:Didn't find the string \"%s\" LINE:%d\n", __func__, alias, __LINE__);
		free(buffer);
		return -1;
	}
	memset(alias, '\0', strlen(alias));
	sprintf(alias, "</%s>", node_name);
	ptr2 = strstr(ptr2+1, alias); //���ҵ�һ���ڵ��β��
	if(ptr2 == NULL)
	{
		trace(DEBUG, "[%s]:Didn't find the string \"%s\" LINE:%d\n", __func__, alias, __LINE__);
		free(buffer);
		return -1;
	}
	len1 = strlen(ptr1);
	len2 = strlen(ptr2);
	len3 = len1 - len2 + strlen((char*)node_name) + 4; //xml�ַ����ĳ���
	if(len1 < len2) //Դ�ַ�����ʽ����
	{
		trace(DEBUG, "[%s]:the format of the source string error! LINE:%d\n", __func__, __LINE__);
		free(buffer);
		return -1;
	}
	if(dest_size <= len3) //�û��������Ļ������ռ䲻��
	{
		trace(DEBUG, "[%s]: Cache space is too small, length should be greater than %d! LINE:%d\n", __func__, len3,__LINE__);
		free(buffer);
		return -1;
	}
	memcpy(dest, (ptr1-1), len3);
	free(buffer);

	return 0;
}

/******************************************************************************
 * Description����http����֡��ȡ��xml���ݶ�
 * Input src��Դ�ַ���
 * Output dest��Ŀ���ַ���
 * Input dest_size��Ŀ���ַ�������ռ�Ĵ�С
 * Return��0-�ɹ� <0-�����
 * *****************************************************************************/
int getXmlDataFromHttp(const void *src, uint8 *dest, int dest_size)
{
	char *ptr1 = NULL;
	char head[1024] = {0};
	int head_len = 0, body_len = 0, data_len = 0, septa_len = 0;

	/*��ȡhttp���ݰ���ͷ�����ķֿ�*/
	ptr1 = strstr(src, "<?xml version=");
	if(ptr1 == NULL)
	{
		trace(TRACE, "[%s]:there is no xml data! LINE:%d\n", __func__, __LINE__);
		return HTTP_WRONG;
	}
	body_len = strlen(ptr1);
	memcpy(dest, ptr1, body_len);
	head_len = strlen(src) - body_len; 
	memcpy(head, src, head_len);

	/*��������֡��ͷ*/
	ptr1 = NULL;
	ptr1 = strstr(head, "Content-Length:");
	if(ptr1 != NULL) //Content-Length���͵�����֡�м�û��trace����
	{
		return 0;
	}

	/*����Ƿ��� chunkeds �ֶ�*/
	ptr1 = NULL;
	ptr1 = strstr(head, "Transfer-Encoding:");
	if(ptr1 == NULL)
	{
		return HTTP_WRONG;
	}

	/*�õ���һ��chunkeds�ĳ���*/
	ptr1 = NULL;
	ptr1 = strstr(head, "\r\n\r\n");
	if(ptr1 == NULL)
	{
		return 0;
	}
	body_len = strtol((char*)ptr1, NULL, 16); //��һ��chunkeds�Ĵ�С 
	
	data_len = 0;
	while(1)
	{
		/*��ȡ��ʾchunkeds���ȵ��ַ����ĳ���*/
		septa_len = 5;
		ptr1 = strstr((char*)&(dest[body_len]), "\r\n");
		if(NULL != ptr1)
		{
			data_len = strlen(ptr1);
			ptr1 = strstr((char*)&(dest[body_len+2]), "\r\n");
			if(NULL != ptr1)
			{
				septa_len = data_len - strlen(ptr1)+2;  //��ʾ chunkeds���ȵ��ַ����ĳ��ȼ����ַ���"\r\n"�ĳ���
			}
		}

		data_len = 0;
		data_len = strtol((char*)&dest[body_len+2], NULL, 16); /*��ȡÿһ�� chunkeds ���ַ����ĳ���*/
		if(data_len == 0)
		{
			memset((char*)&dest[body_len-1], 0, 7); //ȥ������һ��chunkeds�Ľ����ַ���\r\n\r\n0\r\n
			break;
		}

		memcpy(&dest[body_len], &dest[body_len+septa_len], strlen((char*)&(dest[body_len+septa_len]))); //����ʶchunkeds���ȵ��ַ�ȥ��
		body_len += data_len;
	}

	return 0;
}

/******************************************************************************
 * Description�����http����֡����ȷ��
 * Input buffer����Ҫ���м���http����֡
 * Output error_info������� 
 * Return��0-�ɹ� -1-ʧ��
 * *****************************************************************************/
sint32 checkHttpData(const void *buffer)
{
	if(NULL == strstr(buffer, "HTTP/1.1 200 OK"))
	{
		trace(ERROR, "[%s]:current package is not \"200 OK\"! LINE:%d\n", __func__, __LINE__);
		return HTTP_EXTEND;
	}
	
	return 0;
}

/*******************************************************************************
 * Description:����http������������շ������˵Ļظ�
 * Input server:��Ҫ���ж�д���ļ�
 * Input src:���͸�web�������˵�http�����
 * Output dest:web�������˻ظ���http��Ӧ��
 * Output errno:ִ��ʧ�ܵ�ԭ��
 * Return:0-�ɹ��� <0-�������
 * ****************************************************************************/
sint32 read_write_process(NETWORK_ATTR_S *server, const void* src, void *dest, sint32 dest_size)
{
	sint32 ret = 0;
	PACKET_ATTR_S http_attr;
	
	if((server == NULL) || (server->connect != 1))
	{
		trace(ERROR, "[%s]:disconnect the serve! LINE:%d\n");
		return -1;
	}

	/*��Web�������˷���http�����*/
	ret = send_data(&(server->fd), src, strlen(src)); 
	if(ret < 0)
	{
		server->connect = 0;
		trace(ERROR, "[%s]:send_data=send data faild! LINE:%d\n", __func__, __LINE__);
		isapi_set_connect_status(server);
		return SEND_ERROR;
	}
	trace(DEBUG, "\e[32m[%s]:[send data:%d]\e[0m:\n%s LINE:%d\n", __func__, strlen(src), src, __LINE__);

	/*����web�������˻ظ���http���ݰ�*/
	memset(&http_attr, '\0', sizeof(http_attr));
	http_attr.attr = PACKET_ATTR_HEADANDTAIL_E;
	strncpy(http_attr.head, "HTTP/", sizeof(http_attr.head));
	strncpy(http_attr.tail, "\r\n\r\n", sizeof(http_attr.tail));
	ret = receive_data(&(server->fd), &http_attr, dest, dest_size);
	if(ret <= 0)
	{
		server->connect = 0; //�Ͽ�socket����
		trace(ERROR, "[%s]:receive_data = receive data faild! LINE:%d\n", __func__, __LINE__);
		isapi_set_connect_status(server);
		return RECEIVE_ERROR;
	}
	trace(DEBUG, "\e[32m[%s]:[received data:%d]\e[0m:\n%s LINE:%d\n", __func__, strlen(dest), dest, __LINE__);

	return 0;
}

/******************************************************************************
 * Description���������ݵ��շ�����ɵĹ��������㣬��һ���Ǵ������������ӣ��ڶ�����
 * 				�շ�����֡
 * Input tx_buffer�����͵�����
 * Output rx_buffer�����ܵ�����
 * Input rx_len������������Ĵ�С
 * Return��0-�ɹ� <0-�������
 * *****************************************************************************/
sint32 send_receive_data(const void *tx_buffer, void *rx_buffer, const sint32 rx_len)
{
	int ret = 0;
	HTTP_ATTR_S http_attr;
	NETWORK_ATTR_S server;

	/*������Web�������˵�����*/
	memset(&http_attr, 0, sizeof(HTTP_ATTR_S));
	memset(&server, 0, sizeof(NETWORK_ATTR_S));
	isapi_get_http_attr(&http_attr);
	memcpy(server.ip, http_attr.ip, sizeof(http_attr.ip));
	server.port = http_attr.port;
	server.type = http_attr.connection;
	ret = retry_connect_server_block(&server);
	if(ret < 0)
	{
		return ret;
	}

	/*�շ�����֡*/
	ret = read_write_process(&server, tx_buffer, rx_buffer, rx_len);
	if(ret < 0)
	{
		return ret;
	}

	/*���HTTP����֡����ȷ��*/
	ret = checkHttpData(rx_buffer); 
	if(ret < 0)
	{
		trace(TRACE, "[%s]:Wrong http package! LINE:%d\n", __func__, __LINE__);
		return ret;
	}

	return 0;
}

/******************************************************************************
 * Description������һ��PUT HTTP�����
 * Input url��url
 * Input xml��http����֡�ĸ���
 * Output buffer:�����õ�http���ݰ�
 * Input rx_len���������Ĵ�С
 * Return��0-�ɹ� <0-�������
 * *****************************************************************************/
sint32 put_http_package(const void *url, const void *http_body, void *buffer, const int size)
{
	sint32 data_len = 0;
	HTTP_ATTR_S http_attr;
	uint8 http_cmd[2048] = {0}, connection[32] = {0};
	
	memset(&http_attr, 0, sizeof(HTTP_ATTR_S));
	isapi_get_http_attr(&http_attr);

	/*http�����ӷ�ʽ�������ӻ��Ƕ�����*/
	if(http_attr.connection == KEEP_ALIVE)
	{
		sprintf((char*)connection, "keep_alive");
	}
	else
	{
		sprintf((char*)connection, "close");
	}
	sprintf((char*)http_cmd, "PUT %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Connection: %s\r\n"
			"Content-Length: %d\r\n"
			"Cache-Control: max-age=0\r\n"
			"Authorization: Basic YWRtaW46ODg4ODg4\r\n"
			//"Origin: http://%s\r\n"
			"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Accept: application/xml, text/xml, */*; q=0.01\r\n"
			"X-Requested-With: XMLHttpRequest\r\n"
			"If-Modified-Since: 0\r\n"
			//"Referer: http://%s/doc/page/paramconfig.asp\r\n"
			"Accept-Encoding: gzip, deflate, sdch\r\n"
			"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"
			"%s", (char*)url, http_attr.ip, connection, strlen((char*)http_body), (char*)http_body);
			//"%s", (char*)url, http_attr.ip, connection, strlen((char*)http_body), http_attr.ip, http_attr.ip, (char*)http_body);
			//"Cookie: streamType=0; BufferLever=0.1; updateTips=true; userInfo80=YWRtaW46ODg4ODg4; DevID=1; language=zh; curpage=paramconfig.asp%254\r\n"
	data_len = strlen((char*)http_cmd);
	if(size < data_len) //������̫С
	{
		trace(ERROR, "[%s]:Cache space is too small! size=%d(%d) LINE:%d\n", __func__, size, data_len,__LINE__);
		return -1;
	}
	memcpy(buffer, http_cmd, data_len);
	return data_len;
}

/******************************************************************************
 * Description������һ��GET HTTP�����
 * Input url��url
 * Input xml��http����֡�ĸ���
 * Output buffer:�����õ�http���ݰ�
 * Input rx_len���������Ĵ�С
 * Return��0-�ɹ� <0-�������
 * *****************************************************************************/
sint32 get_http_package(const void *url, const void *http_body, void *buffer, const int size)
{
	sint32 data_len = 0;
	HTTP_ATTR_S http_attr;
	uint8 http_cmd[2048] = {0}, connection[32] = {0};

	memset(&http_attr, 0, sizeof(HTTP_ATTR_S));
	isapi_get_http_attr(&http_attr);

	/*http�����ӷ�ʽ�������ӻ��Ƕ�����*/
	if(http_attr.connection == KEEP_ALIVE)
	{
		sprintf((char*)connection, "keep_alive");
	}
	else
	{
		sprintf((char*)connection, "close");
	}

	sprintf((char*)http_cmd, "GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Connection: %s\r\n"
			"Cache-Control: max-age=0\r\n"
			"Accept: */*\r\n"
			"X-Requested-With: XMLHttpRequest\r\n"
			"If-Modified-Since: 0\r\n"
			"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n"
			"Authorization: Basic YWRtaW46ODg4ODg4\r\n"
			//"Referer: http://%s/doc/page/paramconfig.asp\r\n"
			"Accept-Encoding: gzip, deflate, sdch\r\n"
			"Accept-Language: zh-CN,zh;q=0.8\r\n" "\r\n" , (char*)url, connection, http_attr.ip);
	//"%s", (char*)url, http_attr.ip, connection, strlen((char*)http_body), (char*)http_body);
	data_len = strlen((char*)http_cmd);
	if(size < data_len) //������̫С
	{
		trace(ERROR, "[%s]:Cache space is too small! size=%d(%d) LINE:%d\n", __func__, size, data_len,__LINE__);
		return -1;
	}
	memcpy(buffer, http_cmd, data_len);
	return data_len;
}

/******************************************************************************
 * Description������url������ͬ��HTTP���ݰ�
 * Input methods������
 * Input url��url
 * Input xml��http����֡�ĸ���
 * Output buffer:�����õ�http���ݰ�
 * Input rx_len���������Ĵ�С
 * Return��0-�ɹ� <0-�������
 * *****************************************************************************/
sint32 http_package(HTTP_METHODS_E methods,  const void *url, const void *tx_data, void *rx_data, const int data_len)
{
	int ret = 0, tx_len = 2048, rx_len = 40960;
	uint8 *tx_buffer = NULL, *rx_buffer = NULL;

	/*���������������ռ�*/
	tx_buffer = malloc(tx_len);
	if(tx_buffer == NULL)
	{
		trace(ERROR, "[%s]:malloc=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return MEMORY_FAILURE;
	}
	memset(tx_buffer, 0, tx_len);

	rx_buffer = malloc(rx_len);
	if(rx_buffer == NULL)
	{
		trace(ERROR, "[%s]:malloc=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return MEMORY_FAILURE;
	}
	memset(rx_buffer, 0, rx_len);

	/*����http�����*/
	switch(methods)
	{
	case HTTP_GET: //GET
		ret = get_http_package(url, tx_data, tx_buffer, tx_len);
		break;
	case HTTP_PUT: //PUT
		ret = put_http_package(url, tx_data, tx_buffer, tx_len);
		break;
	default:
		break;
	}
	if(ret <= 0)
	{
		trace(ERROR, "[%s]:create http package faild! LINE:%d\n", __func__, __LINE__);
		free(tx_buffer);
		free(rx_buffer);
		return HTTP_CREATE;
	}

	/*�շ�http���ݰ�*/
	ret = send_receive_data(tx_buffer, rx_buffer, rx_len); //�շ�http����
	if(ret < 0)
	{
		free(tx_buffer);
		free(rx_buffer);
		return ret;
	}

	getXmlDataFromHttp(rx_buffer, rx_data, data_len); //��http���н�xml�ļ�ȡ��
	free(tx_buffer);
	free(rx_buffer);

	return ret;
}
#endif

