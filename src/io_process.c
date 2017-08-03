/****************************************************************
 * Copyright(C), 2016, Hanbang Tech.Co.,Ltd
 * File Name:slave_http
 * Author:merlin
 * Version:v1
 * Data:20160613
 * Description:这是一个HTTP协议的客户端程序
 * History:
 * *************************************************************/
#include "common.h"

/******************************************************************************
 * Description：读取配置文件，获取http服务器端口号
 * Input path：配置文件路径
 * Output port：端口号
 * Return：0-成功 -1-失败
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
 * Description：检测目标文件是否可读可写
 * Input file：需要检测的文件
 * Return：0-可读 1-可写 -1-超时或错误
 * *****************************************************/
sint32 is_read_write(const sint32 *file) 
{
	sint32 ret = 0;
	struct pollfd fds[1];
	memset(fds, '\0', sizeof(struct pollfd) * 1);
	fds[0].fd = (*file);
	fds[0].events = (POLLIN | POLLOUT);
	ret = poll(fds, 1, 3000); //1000毫秒的超时时间
	if(ret <= 0) //超时或出错
		return -1;
	if(fds[0].revents & POLLIN) //可读
		return 0;
	else if(fds[0].revents & POLLOUT) //可写
		return 1;

	return -1;
}

#if 0
/*******************************************************
 * Description：和TCP服务器端建立一个连接（非阻塞的方式），可以是长连接也可以是短连接
 * Input serer：建立连接需要用的信息
 * Output server：连接都得到的信息
 * Return：0-成功 <0-错误代码
 * *****************************************************/
sint32 connect_server_nonblock(NETWORK_ATTR_S *server)
{
	if(server->connect == 1)
		return 0;
#if 0
	/*检测请求建立连接的时间间隔*/
	long sint32 now = time(NULL);
	long sint32 sint32erval = now - (history.btime);
	//server->btime = time(NULL);
	trace(DEBUG, "[%s]:sint32erval=%ld now=%ld server->btime=%ld LINE:%d\n", __func__, sint32erval, now, history.btime, __LINE__);
	//if(sint32erval < 5)
	if(sint32erval < 2)
		return 0;
#endif
	sint32 ret = 0;
	/*创建套接字*/
	if(server->fd > 0)
		close(server->fd);
	server->fd = 0;
	server->fd = createSocket(AF_INET, SOCK_STREAM, 0);
	if(server->fd < 0){
		trace(ERROR, "[%s]:createSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	/*请求建立连接*/
	ret = connectSocket(&(server->fd), AF_INET, server->ip, server->port);
	if(ret < 0){
		trace(ERROR, "[%s]:connectSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	/*设置成功建立连接的标志*/
	server->connect = 1;
	server->btime = time(NULL);

	return 0;
}


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

	/*如果IP改变必须建立新的连接*/
	memset(&history, 0, sizeof(history));
	isapi_get_connect_status(&history);
	trace(DEBUG, "[%s]:history connect info fd=%d ip=%s port=%d connect=%d type=%d LINE:%d\n", __func__, history.fd, history.ip, history.port, history.connect, history.type, __LINE__);
	if(0 != strncmp(history.ip, server->ip, strlen(history.ip))) //IP是否改变
	{
		history.connect = 0;
		close(history.fd);
	}

	/*如果是短连接，建立新的连接*/
	if(server->type != KEEP_ALIVE)
	{
		history.connect = 0;
	}

	if(history.connect == 1) //连接状态
	{
		(*server) = history;
		return 0;
	}

	if(server->fd > 0)
	{
		close(server->fd);
	}
	server->fd = 0;
	
	/*创建套接字*/
	server->btime = time(NULL);
	server->fd = createSocket(AF_INET, SOCK_STREAM, 0);
	if(server->fd < 0)
	{
		trace(ERROR, "[%s]:createSocket excuate faild! LINE:%d\n", __func__, __LINE__);
		return CREATE_SOCKET;
	}
	/*请求建立连接*/
	trace(TRACE, "[%s]:try connect to server->ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
	ret = connectSocket(&(server->fd), AF_INET, server->ip, server->port);
	if(ret < 0)
	{
		trace(ERROR, "[%s]:connectSocket excuate faild! ip=%s port=%d LINE:%d\n", __func__, server->ip, server->port, __LINE__);
		return CONNECT_FAILED;
	}
	/*设置成功建立连接的标志*/
	server->connect = 1;
	memset(&history, 0, sizeof(history));
	history = (*server);
	isapi_set_connect_status(&history);

	return 0;
}
#endif

/*
 * 函数功能说明：发送数据
 * 输入参数：需要发送的数据帧
 * 输出参数：
 * 函数返回值：
 * */
/******************************************************************************
 * Description：将数据写到目标文件中
 * Input file：目标文件
 * Input data：需要写入文件的数据
 * Input size：数据的大小
 * Return：0-成功 -1-失败
 * *****************************************************************************/
sint32 send_data(const sint32 *file, const void *data, const sint32 size)
{
	if((*file < 0) || (size <= 0))
	{
		return -1;
	}

	sint32 ret = 0;

	/*检测文件是否可写*/
	if(1 != is_read_write(file))
	{
		trace(ERROR, "[%s]:the targer file cannot be written! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	/*发送数据*/
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
 * Description：从目标文件上读取数据
 * Input file：需要读取的文件
 * Input size：缓存去区的大小
 * Output data：读取到的数据
 * Return：-1:出错 !=-1:读取到的数据帧的长度
 * *****************************************************************************/
sint32 read_data(const sint32 *file, void *data, sint32 size)
{
	if((*file < 0) || (size <= 0))
		return -1;

	sint32 ret = 0;
	
	/*检测文件是否可读*/
	if(0 != is_read_write(file)) 
	{
		usleep(1000);
		trace(ERROR, "[%s]:the targer file cannot read! file=%d LINE:%d\n", __func__, (*file), __LINE__);
		return -1;
	}
	/*读取文件中的数据*/
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
 * Description：检查目标数据帧
 * Input buffer：需要进行检测的数据帧
 * Input attr：数据帧的属性描述
 * Return：0-成功 -1-未检测到头 -2-未检测到尾 -3-未检测到特殊的字符串  -4-数据帧的长度不够
 * *****************************************************************************/
sint32 packet_check(const void *buffer, PACKET_ATTR_S *attr)
{
	switch(attr->attr)
	{
	case PACKET_ATTR_HEADANDTAIL_E: //head and tail
		if(NULL == strstr(buffer, attr->head))
			return PACKET_ATTR_HEAD_E; //未检测到头
		if(NULL == strstr(buffer, attr->tail))
			return PACKET_ATTR_TAIL_E; //未检测到尾
		break;
	case PACKET_ATTR_HEAD_E: //only the head
		if(NULL == strstr(buffer, attr->tail))
			return  PACKET_ATTR_HEAD_E; //未检测到头
		break;
	case PACKET_ATTR_SPECIAL_E: //only special string 
		if(NULL == strstr(buffer, attr->special))
			return PACKET_ATTR_SPECIAL_E; //未检测带特殊的字符串
		break;
	case PACKET_ATTR_LEN_E: //fixed length
		if(attr->len > strlen(buffer))
			return PACKET_ATTR_LEN_E; //长度不够
		break;
	default:
		break;
	}
	return 0;
}

#if 0
/******************************************************************************
 * Description：从目标文件上读取数据
 * Input file：需要读取的文件
 * Input attr：数据包属性描述
 * Input size：缓存去区的大小
 * Output data：读取到的数据
 * Output：-1:出错 !=-1:读取到的数据帧的长度
 * *****************************************************************************/
sint32 receive_data(const sint32 *file, PACKET_ATTR_S *attr, void *data, sint32 size)
{	
	if((file == NULL) || (data == NULL) || (size < 0))
	{
		trace(ERROR, "[%s]:param wrong! LINE:%d\n", __func__, __LINE__);
		return -1;
	}

	sint32 i = 0, ret = 0, data_len1 = 0, data_len2 = 0;
	for(i = 0; i < 3; i++) //如果读不到，循环读取三次
	{
		data_len1 = read_data(file, data, size);
		if(data_len1 < 0)
		{
			trace(TRACE, "[%s]:timeout handler! LINE:%d\n", __func__, __LINE__);
			continue;
		}
		break;
	}
	
	/*读了三次都没有读到，当超时处理*/
	if(i >= 3)
	{
		trace(ERROR, "[%s]:read timeout! LINE:%d\n", __func__, __LINE__);
		return -1;
	}
	
	/*检测读到的数据帧书否完整*/
	ret = packet_check(data, attr);
	if(ret >= 0) //收到一个完整的数据帧
		return strlen(data);

	sint32 buffer_size = 10240;
	void *buffer = (void*)malloc(buffer_size);
	if(buffer == NULL)
	{
		trace(ERROR, "[%s]:malloc=%s LINE:%d\n", __func__, strerror(errno), __LINE__);
		return -1;
	}
	memset(buffer, '\0', buffer_size);
	
	/*收到的数据帧不完整需要进行分包处理*/
	data_len1 = strlen(data); //第一包数据的长度
	trace(DEBUG, "[%s]:deal with subpackage. LINE:%d\n", __func__, __LINE__);
	if(ret == PACKET_ATTR_TAIL_E) //分包处理
	{
		ret = read_data(file, buffer, buffer_size);
		if(ret < 0)
		{
			trace(ERROR, "[%s]:Didn't receive data! LINE;%d\n", __func__, __LINE__);
			free(buffer);
			return strlen(data);
		}
	}

	/*收到一个新的数据包*/
	data_len2 = strlen(buffer); //第二包数据的长度
	if(NULL != strstr(buffer, attr->head))
	{
		trace(TRACE, "[%s]:receive new package! LINE:%d\n", __func__, __LINE__);
		memcpy(data, buffer, data_len2);
		free(buffer);
		return data_len2;
	}

	/*拼接数据包*/
	if((size-1) < (data_len1+data_len2)) //缓存区小了
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
 * Description：从http数据帧中取出xml数据段，此方法必须要知道xml数据帧的根节点名称
 * Input src：源字符串
 * Output dest：目标字符串
 * Input dest_size：目标字符串缓存空间的大小
 * Input node_name：xml文件第一个节点的节点名称，注意此节点的字符串必须是唯一的
 * Return：0-成功 -1-失败
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
	
	/*检测xml文件的完整性*/
	ptr1 = strstr(buffer, "?xml version="); //查找xml文件头
	if(ptr1 == NULL)
	{
		trace(DEBUG, "[%s]:Didn't find the string \"?xml version=\"! LINE:%d\n", __func__, __LINE__);
		free(buffer);
		return -1;
	}
	memset(alias, '\0', strlen(alias));
	sprintf(alias, "<%s", node_name);
	ptr2 = strstr(ptr1, alias); //查找第一个节点的头部
	if(ptr2 == NULL)
	{
		trace(DEBUG, "[%s]:Didn't find the string \"%s\" LINE:%d\n", __func__, alias, __LINE__);
		free(buffer);
		return -1;
	}
	memset(alias, '\0', strlen(alias));
	sprintf(alias, "</%s>", node_name);
	ptr2 = strstr(ptr2+1, alias); //查找第一个节点的尾部
	if(ptr2 == NULL)
	{
		trace(DEBUG, "[%s]:Didn't find the string \"%s\" LINE:%d\n", __func__, alias, __LINE__);
		free(buffer);
		return -1;
	}
	len1 = strlen(ptr1);
	len2 = strlen(ptr2);
	len3 = len1 - len2 + strlen((char*)node_name) + 4; //xml字符串的长度
	if(len1 < len2) //源字符串格式错误
	{
		trace(DEBUG, "[%s]:the format of the source string error! LINE:%d\n", __func__, __LINE__);
		free(buffer);
		return -1;
	}
	if(dest_size <= len3) //用户传过来的缓存区空间不够
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
 * Description：从http数据帧中取出xml数据段
 * Input src：源字符串
 * Output dest：目标字符串
 * Input dest_size：目标字符串缓存空间的大小
 * Return：0-成功 <0-错误号
 * *****************************************************************************/
int getXmlDataFromHttp(const void *src, uint8 *dest, int dest_size)
{
	char *ptr1 = NULL;
	char head[1024] = {0};
	int head_len = 0, body_len = 0, data_len = 0, septa_len = 0;

	/*获取http数据包的头和正文分开*/
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

	/*解析数据帧的头*/
	ptr1 = NULL;
	ptr1 = strstr(head, "Content-Length:");
	if(ptr1 != NULL) //Content-Length类型的数据帧中间没有trace数据
	{
		return 0;
	}

	/*检查是否有 chunkeds 字段*/
	ptr1 = NULL;
	ptr1 = strstr(head, "Transfer-Encoding:");
	if(ptr1 == NULL)
	{
		return HTTP_WRONG;
	}

	/*得到第一快chunkeds的长度*/
	ptr1 = NULL;
	ptr1 = strstr(head, "\r\n\r\n");
	if(ptr1 == NULL)
	{
		return 0;
	}
	body_len = strtol((char*)ptr1, NULL, 16); //第一段chunkeds的大小 
	
	data_len = 0;
	while(1)
	{
		/*获取表示chunkeds长度的字符串的长度*/
		septa_len = 5;
		ptr1 = strstr((char*)&(dest[body_len]), "\r\n");
		if(NULL != ptr1)
		{
			data_len = strlen(ptr1);
			ptr1 = strstr((char*)&(dest[body_len+2]), "\r\n");
			if(NULL != ptr1)
			{
				septa_len = data_len - strlen(ptr1)+2;  //表示 chunkeds长度的字符串的长度加上字符串"\r\n"的长度
			}
		}

		data_len = 0;
		data_len = strtol((char*)&dest[body_len+2], NULL, 16); /*获取每一个 chunkeds 块字符串的长度*/
		if(data_len == 0)
		{
			memset((char*)&dest[body_len-1], 0, 7); //去除最后的一块chunkeds的结束字符串\r\n\r\n0\r\n
			break;
		}

		memcpy(&dest[body_len], &dest[body_len+septa_len], strlen((char*)&(dest[body_len+septa_len]))); //将标识chunkeds长度的字符去除
		body_len += data_len;
	}

	return 0;
}

/******************************************************************************
 * Description：检测http数据帧的正确性
 * Input buffer：需要进行检测的http数据帧
 * Output error_info：检测结果 
 * Return：0-成功 -1-失败
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
 * Description:发送http请求包，并接收服务器端的回复
 * Input server:需要进行读写的文件
 * Input src:发送给web服务器端的http请求包
 * Output dest:web服务器端回复的http响应包
 * Output errno:执行失败的原因
 * Return:0-成功； <0-错误代码
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

	/*向Web服务器端发送http请求包*/
	ret = send_data(&(server->fd), src, strlen(src)); 
	if(ret < 0)
	{
		server->connect = 0;
		trace(ERROR, "[%s]:send_data=send data faild! LINE:%d\n", __func__, __LINE__);
		isapi_set_connect_status(server);
		return SEND_ERROR;
	}
	trace(DEBUG, "\e[32m[%s]:[send data:%d]\e[0m:\n%s LINE:%d\n", __func__, strlen(src), src, __LINE__);

	/*接收web服务器端回复的http数据包*/
	memset(&http_attr, '\0', sizeof(http_attr));
	http_attr.attr = PACKET_ATTR_HEADANDTAIL_E;
	strncpy(http_attr.head, "HTTP/", sizeof(http_attr.head));
	strncpy(http_attr.tail, "\r\n\r\n", sizeof(http_attr.tail));
	ret = receive_data(&(server->fd), &http_attr, dest, dest_size);
	if(ret <= 0)
	{
		server->connect = 0; //断开socket连接
		trace(ERROR, "[%s]:receive_data = receive data faild! LINE:%d\n", __func__, __LINE__);
		isapi_set_connect_status(server);
		return RECEIVE_ERROR;
	}
	trace(DEBUG, "\e[32m[%s]:[received data:%d]\e[0m:\n%s LINE:%d\n", __func__, strlen(dest), dest, __LINE__);

	return 0;
}

/******************************************************************************
 * Description：进行数据的收发，完成的工作有两点，第一点是创建并管理连接，第二点是
 * 				收发数据帧
 * Input tx_buffer：发送的数据
 * Output rx_buffer：接受的数据
 * Input rx_len：输出缓存区的大小
 * Return：0-成功 <0-错误代码
 * *****************************************************************************/
sint32 send_receive_data(const void *tx_buffer, void *rx_buffer, const sint32 rx_len)
{
	int ret = 0;
	HTTP_ATTR_S http_attr;
	NETWORK_ATTR_S server;

	/*创建和Web服务器端的连接*/
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

	/*收发数据帧*/
	ret = read_write_process(&server, tx_buffer, rx_buffer, rx_len);
	if(ret < 0)
	{
		return ret;
	}

	/*检测HTTP数据帧的正确性*/
	ret = checkHttpData(rx_buffer); 
	if(ret < 0)
	{
		trace(TRACE, "[%s]:Wrong http package! LINE:%d\n", __func__, __LINE__);
		return ret;
	}

	return 0;
}

/******************************************************************************
 * Description：创建一个PUT HTTP请求包
 * Input url：url
 * Input xml：http数据帧的负载
 * Output buffer:创建好的http数据包
 * Input rx_len：输出缓存的大小
 * Return：0-成功 <0-错误代码
 * *****************************************************************************/
sint32 put_http_package(const void *url, const void *http_body, void *buffer, const int size)
{
	sint32 data_len = 0;
	HTTP_ATTR_S http_attr;
	uint8 http_cmd[2048] = {0}, connection[32] = {0};
	
	memset(&http_attr, 0, sizeof(HTTP_ATTR_S));
	isapi_get_http_attr(&http_attr);

	/*http的连接方式，长连接或是短连接*/
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
	if(size < data_len) //缓存区太小
	{
		trace(ERROR, "[%s]:Cache space is too small! size=%d(%d) LINE:%d\n", __func__, size, data_len,__LINE__);
		return -1;
	}
	memcpy(buffer, http_cmd, data_len);
	return data_len;
}

/******************************************************************************
 * Description：创建一个GET HTTP请求包
 * Input url：url
 * Input xml：http数据帧的负载
 * Output buffer:创建好的http数据包
 * Input rx_len：输出缓存的大小
 * Return：0-成功 <0-错误代码
 * *****************************************************************************/
sint32 get_http_package(const void *url, const void *http_body, void *buffer, const int size)
{
	sint32 data_len = 0;
	HTTP_ATTR_S http_attr;
	uint8 http_cmd[2048] = {0}, connection[32] = {0};

	memset(&http_attr, 0, sizeof(HTTP_ATTR_S));
	isapi_get_http_attr(&http_attr);

	/*http的连接方式，长连接或是短连接*/
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
	if(size < data_len) //缓存区太小
	{
		trace(ERROR, "[%s]:Cache space is too small! size=%d(%d) LINE:%d\n", __func__, size, data_len,__LINE__);
		return -1;
	}
	memcpy(buffer, http_cmd, data_len);
	return data_len;
}

/******************************************************************************
 * Description：根据url创建不同的HTTP数据包
 * Input methods：方法
 * Input url：url
 * Input xml：http数据帧的负载
 * Output buffer:创建好的http数据包
 * Input rx_len：输出缓存的大小
 * Return：0-成功 <0-错误代码
 * *****************************************************************************/
sint32 http_package(HTTP_METHODS_E methods,  const void *url, const void *tx_data, void *rx_data, const int data_len)
{
	int ret = 0, tx_len = 2048, rx_len = 40960;
	uint8 *tx_buffer = NULL, *rx_buffer = NULL;

	/*分配输入输出缓存空间*/
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

	/*创建http请求包*/
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

	/*收发http数据包*/
	ret = send_receive_data(tx_buffer, rx_buffer, rx_len); //收发http数据
	if(ret < 0)
	{
		free(tx_buffer);
		free(rx_buffer);
		return ret;
	}

	getXmlDataFromHttp(rx_buffer, rx_data, data_len); //从http包中将xml文件取出
	free(tx_buffer);
	free(rx_buffer);

	return ret;
}
#endif

