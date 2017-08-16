/*只手*/
#include "common.h"

/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdOption(const int fd, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:option cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	sprintf(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\nPublic: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE,GET_PARAMETER,SET_PARAMETER\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(fd, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);
	return iSendDataLen;
}

/*******************************************************************************
* Description:获取用户用于登录的信息
******************************************************************************/
int getLoginInfo(char *src, SDESCRIBE *p_dest)
{
	char *p1 = NULL, *p2 = NULL, *saveptr1 = NULL;
	char username[128] = {0}; 

	/*获取cseq*/
	p2 = strstr(src, "CSeq:");
	if(NULL != p2){
		sscanf(p2, "%*[^:]:%d", &(p_dest->CSeq));
	}
	/*拆分字符串，分隔符是字符',' 从而获取到登陆信息*/
	for(p1 = strtok_r(src, ",", &saveptr1); p1 != NULL; p1 = strtok_r(NULL, ",", &saveptr1)){
		p2 = strstr(p1, "username=");
		if(NULL != p2){
			sscanf(p2, "%*[^=]=%s", (p_dest->username));
		}
		p2 = strstr(p1, "realm=");
		if(NULL != p2){
			sscanf(p2, "%*[^=]=%s", (p_dest->realm));
		}
		p2 = strstr(p1, "nonce=");
		if(NULL != p2){
			sscanf(p2, "%*[^=]=%s", (p_dest->nonce));
		}
		p2 = strstr(p1, "uri=");
		if(NULL != p2){
			sscanf(p2, "%*[^=]=%s", (p_dest->url));
		}
		p2 = strstr(p1, "response=");
		if(NULL != p2){
			sscanf(p2, "%*[^=]=%s", (p_dest->response));
		}
	}
	
	return 0;
}

/**********************************************************************************
* Description:解析describe消息包
************************************************************************************/
int parsePagDescribe(char *src, SDESCRIBE *p_dest)
{
	if(NULL == p_dest){
		trace(ERROR, "[%s]:in param is wrong! line:%d\n", __func__, __LINE__);
		return -1;
	}

	/*时否需要进行超时处理*/
	if(strstr(src, "timeout") != NULL) {
		p_dest->timeout= 1;
	}
	/*url地址*/
	if(NULL == strstr(src, "DESCRIBE rtsp")){
		trace(ERROR, "[%s]:src is wrong line:%d\n", __func__, __LINE__);
		return -1;
	}
	int len = strlen(src); 
	strncpy((p_dest->url), (src+9), len);
	/*用户登陆信息*/
	getLoginInfo(src ,p_dest);
	
	return 0;
}

/************************************************************************************
* Description:创建一个description回复给VLC客户端
************************************************************************************/
int createPackageDescribe(char *dest)
{
	int pagType = 0;
#if 0
	if(0 == pagType){
		/*创建正确的数据包*/
		sprintf((char*)fResponseBuffer,
   	"RTSP/1.0 401 Unauthorized\r\n"
   	"CSeq: %d\r\n"
   	"WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
   	seqid,
   	user_info.realm, aut_info->nonce);
	}
	else{
		/*创建错误的数据包*/
		
	}
#endif

	return 0;
}

/************************************************************************************
* Description:创建一个Describe消息包
************************************************************************************/
int cmdDescribe(const int sock, RTSP_MSG_ATTR *strMsg)
{
	char txBuf[1024] = {0};
	SDESCRIBE describe;
	RTSP_ATTR_SERVER RTSPServer;
	trace(DEBUG, "[%s]:Describe cmd deal with line:%d\n", __func__, __LINE__);

	/*继续数据帧*/
	parsePagDescribe((strMsg->pmsg), &describe);
	getRTSPServerInfo((RTSPServer.userinfo));
	/*打包数据帧*/
	createPackageDescribe(txBuf);
	
	trace(DEBUG, "[%s]:msg=%s line:%d\n", __func__, (strMsg->pmsg), __LINE__);
	return 0;
}


#if 0
/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdDescribe(const int socket, struct sockaddr_in Listen_addr, char *localip , int authencation, RTSP_MSG_ATTR *strMsg)
{
	int                     	iSendDataLen = 0, iSendDataLen2 = 0, i = 0;
	int                     	usStringLen = 0;
	int                     	listen_port;
	int                     	audio_sample = 48000;
	char                    	pSend2[1000];
	char                    	local_ip[16] = {0};
	char                    	client_ip[16];
	char                  	pSend[TX_BUG_LEN] = {0};
	int 				roomid = strMsg->roomid;
	int 				sdpinfo_len = 0;
	char 			sdpinfo[1024] = {0};
//	int				ret = 0;
	char				audio_config[16] = {0};
	int 				have_audio = 0;
//	char 				nonceBuf[33];
	char temp[50] = "d4d6c4e34ca63476c9e012b908a9a258";

	//nslog(NS_INFO,"roomid=%d \n",roomid);
	if(roomid == -1)
	{
		sprintf(pSend, "RTSP/1.0   404   Stream Not Found\r\nCSeq: %d\r\n\r\n", strMsg->uiCseq);
		iSendDataLen = strlen(pSend);
		send(socket, pSend, iSendDataLen, 0);
		//nslog(NS_INFO,"\n%s\n", pSend);
		return -1;
	}
	if(0)
	{
		#if 0
		"RTSP/1.0 401 Unauthorized\r\n"
	   	"CSeq: %s\r\n"
	   	"%s"
	   	"WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
		#endif

	//	setRealmAndRandomNonce(nonceBuf);

		sprintf(pSend,"RTSP/1.0 401 Unauthorized\r\nCseq: %d\r\nWWW-Authenticate: Digest realm=\"zhengyb\", nonce=\"%s\"\r\n\r\n",strMsg->uiCseq,temp);
		iSendDataLen = strlen(pSend);
		//nslog(NS_INFO,"-----------------------------------------------------\n%s\n", pSend);
		send(socket, pSend, iSendDataLen, 0);
		return -1;
	}


	have_audio = rtsp_stream_get_audio_sinfo(roomid, audio_config, &audio_sample);

	if(have_audio < 0) {
		//nslog(NS_ERROR,"Error\n");
	}

	int mult = 0;
	char ip[32] = {0};
	unsigned int vport = 0;
	unsigned int aport = 0;
	rtsp_porting_get_ginfo(&mult, ip, &vport, &aport,roomid);

	if(mult == 0 || strlen(ip) == 0) {
		strcpy(ip, localip);
	}

	sdpinfo_len =  rtsp_stream_get_video_sdp_info(sdpinfo, roomid);
#ifndef GDM_RTSP_SERVER

	if(rtsp_porting_server_need_stop() == 1) {
		sdpinfo_len = -1;
	}

#endif

	if(authencation == -1 || sdpinfo_len == -1 ) {
		//nslog(NS_INFO,"authencation=%d,sdpinfo_len=%d\n", authencation, sdpinfo_len);
		sprintf(pSend, "RTSP/1.0   404   Stream Not Found\r\nCSeq: %d\r\n\r\n", strMsg->uiCseq);
		iSendDataLen = strlen(pSend);
		send(socket, pSend, iSendDataLen, 0);
		//nslog(NS_INFO,"\n%s\n", pSend);
		return -1;
	}

	sprintf(local_ip, "%s", localip);
	memcpy(client_ip, inet_ntoa(Listen_addr.sin_addr), 16);
	listen_port = RTSP_LISTEN_PORT;
	sprintf(pSend2, "v=0\r\no=- 2890844256 2890842807 IN IP4 %s\r\nc=IN IP4 %s\r\nt=0 0 \r\n", local_ip, ip); //inet_ntoa(Listen_addr.sin_addr));
	iSendDataLen2 = strlen(pSend2);

	sprintf(&pSend2[iSendDataLen2], "m=video %u RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\n", vport);
	iSendDataLen2 = strlen(pSend2);

	if(sdpinfo_len != -1) {
		sprintf(&pSend2[iSendDataLen2], "a=fmtp:96 packetization-mode=1;profile-level-id=428028;sprop-parameter-sets=%s\r\n", sdpinfo);
		iSendDataLen2 = strlen(pSend2);
	}

	sprintf(&pSend2[iSendDataLen2], "a=control:rtsp://%s:%d/trackID=%d\r\n", local_ip, listen_port, VIDEO_TRACKID);
	iSendDataLen2 = strlen(pSend2);

	//only the card0 have audio
	if(have_audio >= 0)
	{
		sprintf(&pSend2[iSendDataLen2], "m=audio %u RTP/AVP 97\r\na=rtpmap:97 mpeg4-generic/%d/2\r\n", aport, audio_sample);
		iSendDataLen2 = strlen(pSend2);
		sprintf(&pSend2[iSendDataLen2], "a=fmtp:97 streamtype=5; mode=AAC-hbr; config=%s; SizeLength=13; IndexLength=3; IndexDeltaLength=3\r\na=control:rtsp://%s:%d/trackID=%d\r\n", audio_config, local_ip, listen_port, AUDIO_TRACKID);
		iSendDataLen2 = strlen(pSend2);
	}
	else
	{
			//nslog(NS_ERROR,"ERROR,get audio sdp failed .\n");
	}

	strcpy(pSend, "RTSP/1.0 200 OK\r\nContent-type: application/sdp\r\nServer: RRS 0.1\r\nContent-Length: ");
	iSendDataLen = strlen(pSend);
	usStringLen = ConventNumberToString(&pSend[iSendDataLen], iSendDataLen2);
	strcpy(&pSend[iSendDataLen + usStringLen], "\r\nCache-Control: no-cache\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(socket, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s", pSend);
	send(socket, pSend2, iSendDataLen2, 0);
	//nslog(NS_INFO,"\n%s\n", pSend2);
	return (iSendDataLen + iSendDataLen2);
	return 0;
}
#endif

#if 1
/*********************************************************************************
* Description:建立响应
**********************************************************************************/
static int rtsp_find_trackid(char *buff, int len)
{
	if(buff  == NULL) {
		return -1;
	}

	char *p = NULL;
	char tempbuff[1024] = {0};
	int id = 0;
	sprintf(tempbuff, "%s", buff);
	p = strstr(tempbuff, "/trackID=");
	//nslog(NS_INFO,"\n");

	if(p == NULL) {
		return -1;
	}

	p = p + strlen("/trackID=") ;
	*(p + 1) = '\0';
	id = atoi(p);
	//nslog(NS_INFO,"i find the trackid = %d\n", id);
	return id;
}

/***************************************************************************
* Description:传输方式
*****************************************************************************/
static void PraseSETUPMsg(char *pDataFromClient, int usDataLen, RTSP_MSG_ATTR *strMsg)
{
	int i = 0, j = 0;
	char *p = pDataFromClient;
	strMsg->ucTransNum = 0;

	for(i = 0; i < usDataLen; i++) {
		/*Transport 传输方式TCP or UDP*/
		if(p[i] == 0x54 && p[i + 1] == 0x72 && p[i + 2] == 0x61 && p[i + 3] == 0x6e && p[i + 4] == 0x73 && p[i + 5] == 0x70 &&
		   p[i + 6] == 0x6f && p[i + 7] == 0x72 && p[i + 8] == 0x74 && p[i + 9] == 0x3a) {
			for(j = 0; j < 100; j++) {
				if(p[i + 11 + j] != 0x0a) {
					strMsg->aucTrans[j] = p[i + 11 + j];
					strMsg->ucTransNum++;
				} else {
					strMsg->ucTransNum -= 1;
					return;
				}
			}

			strMsg->ucTransNum -= 1;
			//			PRINTF("g_strMsg.ucTransNum = %d,g_strMsg.aucTrans[0] =%c\n",strMsg->ucTransNum,strMsg->aucTrans[0]);

			return;
		}
	}

	strMsg->ucTransNum -= 1;
	return;
}

static int GetClientInterleaved(char *pDataFromClient, int usDataLen, RTSP_MSG_ATTR *strMsg, int trick_id)
{
	char *p;
	unsigned int i = 0;
	int m=0;
	strMsg->ucClientportLen[trick_id] = 0;

	p=strstr(pDataFromClient,"interleaved");
	while(p[m]!='='||m>usDataLen){m++;}
	m++;
	while(p[m + i] != 0x3b && p[m + i] != 0x0d && p[m + i + 1] != 0x0a) { //not  ";"  "nler"
				strMsg->aucClientInterleaved[trick_id][i] = p[m + i];
				strMsg->ucClientInterleavedLen[trick_id]++;
				i++;
			}
	//	PRINTF("g_strMsg.ucClientportLen=%d,g_strMsg.aucClientport=%c\n",strMsg->ucClientportLen,strMsg->aucClientport[strMsg->timeused][0]);
	if(i < MIN_CLIENT_PORT_BIT) {
		return ERR_CLIENT_MSG_PORT_TOO_SHORT;
	} else {
		return 1;
	}
}

static int GetClientPort(char *pDataFromClient, int usDataLen, RTSP_MSG_ATTR *strMsg, int trick_id)
{
	char *p = pDataFromClient;
	unsigned int i = 0;
	int m;
	strMsg->ucClientportLen[trick_id] = 0;

	for(m = 0; m < usDataLen; m++) {
		if(p[m] == 0x63 && p[m + 1] == 0x6c && p[m + 2] == 0x69 && p[m + 3] == 0x65 && p[m + 4] == 0x6e && p[m + 5] == 0x74 \
		   && p[m + 6] == 0x5f && p[m + 7] == 0x70 && p[m + 8] == 0x6f && p[m + 9] == 0x72 && p[m + 10] == 0x74 && p[m + 11] == 0x3d) { //client_port=
			while(p[m + 12 + i] != 0x3b && p[m + 12 + i] != 0x0d && p[m + 13 + i] != 0x0a) { //not  ";"  "nler"
				strMsg->aucClientport[trick_id][i] = p[m + 12 + i];
				strMsg->ucClientportLen[trick_id]++;
				i++;
			}
		}
	}

	//nslog(NS_INFO,"g_strMsg.ucClientportLen=%d,g_strMsg.aucClientport=%c\n",strMsg->ucClientportLen,strMsg->aucClientport[strMsg->timeused][0]);
	if(i < MIN_CLIENT_PORT_BIT) {
		return ERR_CLIENT_MSG_PORT_TOO_SHORT;
	} else {
		return 0;
	}
}


#endif

/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdSetup(char *pReceiveBuf, int ReceiveBufLen, const int sock, RTSP_MSG_ATTR *strMsg, int timeout)
{
	trace(DEBUG, "[%s]:Setup cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	int is_video = 1;
	int id = 0;
	id = rtsp_find_trackid(pReceiveBuf, ReceiveBufLen);

	if(id == VIDEO_TRACKID) {
		is_video = 1;
	} else if(id == AUDIO_TRACKID) {
		is_video = 2;
	} else {
		is_video = 1;
	}

	PraseSETUPMsg(pReceiveBuf, ReceiveBufLen, strMsg);
	if(strstr((const char *)strMsg->aucTrans,"TCP")!=NULL)
	{
		    strMsg->tcp_or_udp=1;
			GetClientInterleaved(pReceiveBuf, ReceiveBufLen, strMsg, is_video - 1);
	}
	else
	{
		    strMsg->tcp_or_udp=0;
			GetClientPort(pReceiveBuf, ReceiveBufLen, strMsg, is_video - 1);
	}

	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\nTransport: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucTransNum; i++) {
		pSend[iSendDataLen + i] = strMsg->aucTrans[i];
	}

	for(i = 0; i < strMsg->ucClientportLen[strMsg->timeused]; i++) {
		pSend[iSendDataLen + strMsg->ucTransNum + i ] = strMsg->aucClientport[strMsg->timeused][i];
	}

	//need client timeout
	if(timeout != 0) {
		sprintf(&pSend[iSendDataLen + strMsg->ucTransNum + strMsg->ucClientportLen[strMsg->timeused]], "\r\nContent-Length: 0\r\nCache-Control: no-cache\r\nSession: 11478; timeout=%d\r\nCseq: ", timeout);
	} else {
		strcpy(&pSend[iSendDataLen + strMsg->ucTransNum + strMsg->ucClientportLen[strMsg->timeused]], "\r\nContent-Length: 0\r\nCache-Control: no-cache\r\nSession: 11478\r\nCseq: ");
	}

	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);
	strMsg->timeused = strMsg->timeused + 1;

	if(strMsg->timeused == 2) {
		strMsg->timeused = 0;
	}

	return iSendDataLen;
	return 0;
}

/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdPlay(const int sock, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:Play cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\nSession: 11478\r\nRange: npt=0.000- \r\n");
	iSendDataLen = strlen(pSend);

	sprintf(&pSend[iSendDataLen], "Cseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);

	return iSendDataLen;
	return 0;
}

/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdPause(const int sock, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:Pause cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\nSession: 11478\r\n");
	iSendDataLen = strlen(pSend);

	sprintf(&pSend[iSendDataLen], "Cseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);
	return iSendDataLen;
}


/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdAnnounce(const int sock, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:Announce cmd deal with line:%d\n", __func__, __LINE__);
	return 0;
}


/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdTeardown(const int sock, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:Teardown cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\n");
	iSendDataLen = strlen(pSend);


	sprintf(&pSend[iSendDataLen], "Cseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}


	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);
	return iSendDataLen;
	return 0;
}

/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdGetParameter(const int sock, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:GetParameter cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	sprintf(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);
	return iSendDataLen;
	return 0;
}

/****************************************************************************
 * Description:
 ****************************************************************************/
int cmdSetParameter(const int sock, RTSP_MSG_ATTR *strMsg)
{
	trace(DEBUG, "[%s]:SetParameter cmd deal with line:%d\n", __func__, __LINE__);
	int iSendDataLen = 0;
	int i = 0;
	char  pSend[TX_BUG_LEN] = {0};
	sprintf(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	//nslog(NS_INFO,"\n%s\n", pSend);
	return iSendDataLen;
	return 0;
}


/*****************************************************************************
 * Description；处理具体的rtsp命令
 ******************************************************************************/
int dealWirhRtspCmd(RTSP_ATTR *p_clientAttr, RTSP_MSG_ATTR *p_msgAttr)
{
	char localIP[64] = {"192.168.5.192"};
	int timeout = 0;
	int fd = p_clientAttr->fd;
	int cmd = p_msgAttr->iType;
	switch(cmd)
	{
	case OPTION:
		cmdOption(fd, p_msgAttr);
		break;

	case DESCRIBE:
		cmdDescribe(fd, p_msgAttr);
		//cmdDescribe(fd, (p_clientAttr->addr), localIP, 0, p_msgAttr)
		break;
	case SETUP:
		cmdSetup((p_msgAttr->pmsg), strlen(p_msgAttr->pmsg), fd, p_msgAttr, timeout); /*设置会话属性，以及传输模式，提醒server建立会话*/
		break;
	case PLAY:
		cmdPlay(fd, p_msgAttr); /*开始播放*/
		break;
	case PAUSE:
		cmdPause((fd), p_msgAttr); /*暂停播放*/
		break;
	case TEARDOWN:
		cmdTeardown(fd, p_msgAttr); /*请求关闭会话*/
		break;
	case ANNOUNCE:
		cmdAnnounce(fd, p_msgAttr);
		break;
	case GET_PARAMETER:
		cmdGetParameter(fd, p_msgAttr); /*获取参数*/
		break;
	case SET_PARAMETER:
		cmdSetParameter(fd, p_msgAttr); /*设置参数*/
		break;
	default:
		break;
	}

	return 0;
}


