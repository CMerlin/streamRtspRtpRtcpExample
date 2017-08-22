/*
 * RtpMakerH264.cpp
 *
 *  Created on: 2016年8月12日
 *      Author: xzl
 */

#include "Common/config.h"
#include "RtpMakerH264.h"
#include "Util/mini.h"
#include "Network/sockutil.h"

using namespace ZL::Util;
using namespace ZL::Network;

namespace ZL {
namespace Rtsp {

void RtpMaker_H264::makeRtp(const char* pcData, int iLen, uint32_t uiStamp) {
	TraceL<<"[merlin]:Begin"<<endl;
	static uint32_t cycleMS = mINI::Instance()[Config::Rtp::kCycleMS].as<uint32_t>();
	uiStamp %= cycleMS;
	int iSize = m_iMtuSize - 2;
	if (iLen > iSize) { //超过MTU
		const unsigned char s_e_r_Start = 0x80;
		const unsigned char s_e_r_Mid = 0x00;
		const unsigned char s_e_r_End = 0x40;
		//获取帧头数据，1byte
		unsigned char naluType = *((unsigned char *) pcData) & 0x1f; //获取NALU的5bit 帧类型

		unsigned char nal_ref_idc = *((unsigned char *) pcData) & 0x60; //获取NALU的2bit 帧重要程度 00 可以丢 11不能丢
		//nal_ref_idc = 0x60;
		//组装FU-A帧头数据 2byte
		unsigned char f_nri_type = nal_ref_idc + 28;//F为0 1bit,nri上面获取到2bit,28为FU-A分片类型5bit
		unsigned char s_e_r_type = naluType;
		bool bFirst = true;
		bool mark = false;
		int nOffset = 1;
		while (!mark) {
			if (iLen < nOffset + iSize) {			//是否拆分结束
				iSize = iLen - nOffset;
				mark = true;
				s_e_r_type = s_e_r_End + naluType;
			} else {
				if (bFirst == true) {
					s_e_r_type = s_e_r_Start + naluType;
					bFirst = false;
				} else {
					s_e_r_type = s_e_r_Mid + naluType;
				}
			}
			memcpy(aucSectionBuf, &f_nri_type, 1);
			memcpy(aucSectionBuf + 1, &s_e_r_type, 1);
			memcpy(aucSectionBuf + 2, (unsigned char *) pcData + nOffset, iSize);
			nOffset += iSize;
			makeH264Rtp(aucSectionBuf, iSize + 2, mark, uiStamp);  /*NUAL分片组装在RTP数据中*/
		}
	} else {
		makeH264Rtp(pcData, iLen, true, uiStamp); /*NUAL不分片*/
	}
}

inline int RtpMaker_H264::showRtPHeadInfo(unsigned char *data, int size)
{
	if (size < 12) {  
		return -1;  
	}  
	unsigned int version = (data[0] >> 6); /*版本*/   
	unsigned int numCSRCs = data[0] & 0x0f;  
	unsigned int payloadOffset = 12 + 4 * numCSRCs;    
	unsigned int rtpTime = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7]; /*时间戳*/
	unsigned int srcId = data[8] << 24 | data[9] << 16 | data[10] << 8 | data[11];  
	unsigned int seqNum = data[2] << 8 | data[3];  
	printf("[%s]:seqNum=%d version=%d numCSRCs=%d payloadOffset=%d rtpTime=%d srcId=%d line:%d\n", __func__, seqNum, version, numCSRCs, payloadOffset, rtpTime, srcId, __LINE__);

	return 0;  
}

inline void RtpMaker_H264::makeH264Rtp(const void* data, unsigned int len, bool mark, uint32_t uiStamp) {
	TraceL<<"[merlin]:Begin"<<endl;
	uint16_t ui16RtpLen = len + 12;
	m_ui32TimeStamp = (m_ui32SampleRate / 1000) * uiStamp;
	uint32_t ts = htonl(m_ui32TimeStamp);
	uint16_t sq = htons(m_ui16Sequence);
	uint32_t sc = htonl(m_ui32Ssrc);

	auto pRtppkt = obtainPkt();
	auto &rtppkt = *(pRtppkt.get());
	unsigned char *pucRtp = rtppkt.payload;
	/*RTP数据的头*/
	pucRtp[0] = '$';
	pucRtp[1] = m_ui8Interleaved;
	pucRtp[2] = ui16RtpLen >> 8;
	pucRtp[3] = ui16RtpLen & 0x00FF;
	pucRtp[4] = 0x80;
	pucRtp[5] = (mark << 7) | m_ui8PlayloadType;
	memcpy(&pucRtp[6], &sq, 2);
	memcpy(&pucRtp[8], &ts, 4);
	//ssrc
	memcpy(&pucRtp[12], &sc, 4);
	memcpy(&pucRtp[16], data, len);
	rtppkt.PT = m_ui8PlayloadType;
	rtppkt.interleaved = m_ui8Interleaved;
	rtppkt.mark = mark;
	rtppkt.length = len + 16;
	rtppkt.sequence = m_ui16Sequence;
	rtppkt.timeStamp = m_ui32TimeStamp;
	rtppkt.ssrc = m_ui32Ssrc;
	rtppkt.type = TrackVideo;
	uint8_t type = ((uint8_t *) (data))[0] & 0x1F;
	memcpy(rtppkt.payload + 16, data, len);
	onMakeRtp(pRtppkt, type == 5);
	m_ui16Sequence++;
#if 1 /*打印RTP数据包的一些关键信息*/
	TraceL<<"[Merlin]:m_ui16Sequence="<<m_ui16Sequence<<" PT="<<(rtppkt.PT)<<" m_ui8PlayloadType="<<(m_ui8PlayloadType)<<" length="<<(rtppkt.length)<<" mark="<<(rtppkt.mark)<<" m_ui8Interleaved="<<(m_ui8Interleaved)<<" ui16RtpLen="<<(ui16RtpLen)<<endl;
	unsigned char buffer[64] = {0};
	memcpy(buffer, pucRtp, 64);
	showRtPHeadInfo(buffer, 64);
#endif
	//InfoL<<timeStamp<<" "<<time<<" "<<sampleRate;
}

} /* namespace RTP */
} /* namespace ZL */
