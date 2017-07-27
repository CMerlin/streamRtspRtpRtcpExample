#include "common.h"

/***************************************************************************************
* Description:判断用户发送过来的消息类型?
* Input pDataFromClient:消息包，里面含有用户的消息类型
* Input usDataLen:消息的长度
* Return:消息的类型
****************************************************************************************/
static int GetMsgType(char *pDataFromClient, int usDataLen)
{
	char *p = pDataFromClient;
	int itype = -1;
	int m;

	for(m = 0; m < usDataLen - 7; m++) {
		if(p[m] == 0x4f && p[m + 1] == 0x50 && p[m + 2] == 0x54 && p[m + 3] == 0x49 \
		   && p[m + 4] == 0x4f && p[m + 5] == 0x4e) { //receive option
			itype = OPTION;
			break;
		} else if(p[m] == 0x44 && p[m + 1] == 0x45 && p[m + 2] == 0x53 && p[m + 3] == 0x43 \
		          && p[m + 4] == 0x52 && p[m + 5] == 0x49 && p[m + 6] == 0x42 && p[m + 7] == 0x45) {
			itype = DESCRIBE;
			break;
		} else if(p[m] == 0x53 && p[m + 1] == 0x45 && p[m + 2] == 0x54 && p[m + 3] == 0x55 \
		          && p[m + 4] == 0x50) {
			itype = SETUP;
			break;
		} else if(p[m] == 0x50 && p[m + 1] == 0x4c && p[m + 2] == 0x41 && p[m + 3] == 0x59) {
			itype = PLAY;
			break;
		} else if(p[m] == 0x50 && p[m + 1] == 0x41 && p[m + 2] == 0x55 && p[m + 3] == 0x53 \
		          && p[m + 4] == 0x45) {
			itype = PAUSE;
			break;
		} else if(p[m] == 0x54 && p[m + 1] == 0x45 && p[m + 2] == 0x41 && p[m + 3] == 0x52 \
		          && p[m + 4] == 0x44 && p[m + 5] == 0x4f && p[m + 6] == 0x57 && p[m + 7] == 0x4e) {
			itype = TEARDOWN;
			break;
		} //GET_PARAMETER
		else if(p[m] == 'G' && p[m + 1] == 'E' && p[m + 2] == 'T' && p[m + 3] == '_' \
		        && p[m + 4] == 'P' && p[m + 5] == 'A' && p[m + 6] == 'R' && p[m + 7] == 'A'\
		        && p[m + 8] == 'M' && p[m + 9] == 'E' && p[m + 10] == 'T' && p[m + 11] == 'E' && p[m + 12] == 'R') {
			itype = GET_PARAMETER;
			break;
		} //SET_PARAMETER
		else if(p[m] == 'S' && p[m + 1] == 'E' && p[m + 2] == 'T' && p[m + 3] == '_' \
		        && p[m + 4] == 'P' && p[m + 5] == 'A' && p[m + 6] == 'R' && p[m + 7] == 'A'\
		        && p[m + 8] == 'M' && p[m + 9] == 'E' && p[m + 10] == 'T' && p[m + 11] == 'E' && p[m + 12] == 'R') {
			itype = SET_PARAMETER;
			break;
		} else {
			;
		}
	}

	return itype;

}

/******************************************************************************************
* Description:
* Inparam pDataFromClient:客户端发送过来的RTSP数据帧
* Outparam strMsg:解析RTSP报文获取到的数据
* Return:
********************************************************************************************/
static int GetCSeq(char *pDataFromClient, int usDataLen, RTSP_MSG_ATTR *strMsg)
{
	char *p = pDataFromClient;
	int i = 0;
	char aucCSeq[10];
	unsigned int uiCSeq = 0;
	int m, j;

	for(m = 0; m < usDataLen; m++) {
		if(p[m] == 0x43 && p[m + 1] == 0x53 && p[m + 2] == 0x65 && p[m + 3] == 0x71 \
		   && p[m + 4] == 0x3a) { //CSeq
			while(((m + 6 + i) < usDataLen) && (p[m + 6 + i] != 0x0a)) {
				aucCSeq[i] = p[m + 6 + i] - 0x30;
				strMsg->aucCSeqSrc[i] = p[m + 6 + i];
				i++;
			}

			strMsg->ucCSeqSrcLen = i - 1;

			for(j = 0; j < i - 1; j++) {
				//uiCSeq += aucCSeq[j] * pow(10, (i - 2 - j));
				//uiCSeq += (unsigned int)(aucCSeq[j] * pow(10, (i - 2 - j))); /*pow函数总是导致无法编译 merlin_error*/
			}

			break;
		}
	}

	//	PRINTF("uicseq = %d\n",uiCSeq);
	return uiCSeq;
}


/*************************************************************************************************
* Description:获取RTSP消息相关的信息
*************************************************************************************************/
static int GetVlcMsgType(char *pDataFromClient, int usDataLen, RTSP_MSG_ATTR *strMsg)
{
	int type;
	strMsg->iType = GetMsgType(pDataFromClient, usDataLen); /*消息类型*/
	strMsg->uiCseq = GetCSeq(pDataFromClient, usDataLen, strMsg);

	if(strMsg->iType >= RTSP_CMD_MAX|| strMsg->uiCseq > 65535) {
		return 0;
	}

	type = strMsg->iType;
	return type;
}


/************************************************************************************
* Description:解析收到的rtsp命令
*************************************************************************************/
int parseVLCMsg(char *buffer, RTSP_MSG_ATTR *p_msgAttr)
{
	int ret = 0, bufLen = 0;

	if((NULL==buffer) || (NULL==p_msgAttr)){
		trace(ERROR, "[%s]:in param is wrong! line:%d\n", __func__, __LINE__);
		return -1;
	}

	memset(&p_msgAttr, 0, sizeof(RTSP_MSG_ATTR));
	bufLen = strlen(buffer);
	ret = GetVlcMsgType(buffer, bufLen, p_msgAttr);
	
	return 0;
}

