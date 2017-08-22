/*
 * RtspSession.h
 *
 *  Created on: 2016年8月12日
 *      Author: xzl
 */

#ifndef SESSION_RTSPSESSION_H_
#define SESSION_RTSPSESSION_H_

#include <set>
#include <vector>
#include <unordered_map>
#include "Common/config.h"
#include "Rtsp.h"
#include "RtpBroadCaster.h"
#include "RtspMediaSource.h"
#include "Player/PlayerBase.h"
#include "Util/util.h"
#include "Util/logger.h"
#include "Network/TcpLimitedSession.h"

using namespace std;
using namespace ZL::Util;
using namespace ZL::Rtsp;
using namespace ZL::Player;
using namespace ZL::Network;

namespace ZL {
namespace Rtsp {

class RtspSession;
class RtspSession: public TcpLimitedSession<MAX_TCP_SESSION> {
public:
	typedef std::shared_ptr<RtspSession> Ptr;
	RtspSession(const std::shared_ptr<ThreadPool> &pTh, const Socket::Ptr &pSock);
	virtual ~RtspSession();
	void onRecv(const Socket::Buffer::Ptr &pBuf) override;
	void onError(const SockException &err) override;
	void onManager() override;
private:
	typedef bool (RtspSession::*rtspCMDHandle)();
	int send(const string &strBuf) override {
		return m_pSender->send(strBuf);
	}
	int send(const char *pcBuf, int iSize) override {
		return m_pSender->send(pcBuf, iSize);
	}
	void shutdown() override;
	bool handleReq_Options(); //处理options方法
	bool handleReq_Describe(); //处理describe方法
	bool handleReq_Setup(); //处理setup方法
	bool handleReq_Play(); //处理play方法
	bool handleReq_Pause(); //处理pause方法
	bool handleReq_Teardown(); //处理teardown方法
	bool handleReq_Get(); //处理Get方法
	bool handleReq_Post(); //处理Post方法
	bool handleReq_SET_PARAMETER(); //处理SET_PARAMETER方法

	void inline send_StreamNotFound(); //rtsp资源未找到
	void inline send_UnsupportedTransport(); //不支持的传输模式
	void inline send_SessionNotFound(); //会话id错误
	void inline send_NotAcceptable(); //rtsp同时播放数限制
	inline bool findStream(); //根据rtsp url查找 MediaSource实例

	inline void initSender(const std::shared_ptr<RtspSession> &pSession); //处理rtsp over http，quicktime使用的
	inline void sendRtpPacket(const RtpPacket &pkt);
	inline string printSSRC(uint32_t ui32Ssrc) {
		char tmp[9] = { 0 };
		ui32Ssrc = htonl(ui32Ssrc);
		uint8_t *pSsrc = (uint8_t *) &ui32Ssrc;
		for (int i = 0; i < 4; i++) {
			sprintf(tmp + 2 * i, "%02X", pSsrc[i]);
		}
		return tmp;
	}
	inline int getTrackIndexByTrackId(int iTrackId) {
		for (unsigned int i = 0; i < m_uiTrackCnt; i++) {
			if (iTrackId == m_aTrackInfo[i].trackId) {
				return i;
			}
		}
		return -1;
	}
	inline void onRcvPeerUdpData(int iTrackIdx, const Socket::Buffer::Ptr &pBuf, const struct sockaddr &addr);
	inline void tryGetPeerUdpPort();

	char *m_pcBuf = nullptr;
	Ticker m_ticker;
	Parser m_parser; //rtsp解析类,解析出来的RTSP信息会存储在此对象中
	string m_strUrl; /*客户端请求的url*/
	string m_strSdp;
	string m_strSession;
	bool m_bFirstPlay = true;
	string m_strApp;
	string m_strStream;
	std::weak_ptr<RtspMediaSource> m_pMediaSrc;
	static unordered_map<string, rtspCMDHandle> g_mapCmd;

	//RTP缓冲
	weak_ptr<RingBuffer<RtpPacket::Ptr> > m_pWeakRing;
	RingBuffer<RtpPacket::Ptr>::RingReader::Ptr m_pRtpReader;

	PlayerBase::eRtpType m_rtpType = PlayerBase::RTP_UDP;
	bool m_bSetUped = false;
	int m_iCseq = 0; /*消息号*/
	unsigned int m_uiTrackCnt = 0; //媒体track个数
	RtspTrack m_aTrackInfo[2]; //媒体track信息,trackid idx 为数组下标

#ifdef RTSP_SEND_RTCP
	RtcpCounter m_aRtcpCnt[2]; //rtcp统计,trackid idx 为数组下标
	Ticker m_aRtcpTicker[2]; //rtcp发送时间,trackid idx 为数组下标
	inline void sendRTCP();
#endif

	//RTP over UDP
	bool m_abGotPeerUdp[2] = { false, false }; //获取客户端udp端口计数
	weak_ptr<Socket> m_apUdpSock[2]; //发送RTP的UDP端口,trackid idx 为数组下标
	std::shared_ptr<struct sockaddr> m_apPeerUdpAddr[2]; //播放器接收RTP的地址,trackid idx 为数组下标
	bool m_bListenPeerUdpPort = false;
	RtpBroadCaster::Ptr m_pBrdcaster;

	//RTSP over HTTP
	function<void(void)> m_onDestory;
	bool m_bBase64need = false; //是否需要base64解码
	Socket::Ptr m_pSender; //回复rtsp时走的tcp通道，供quicktime用
	//quicktime 请求rtsp会产生两次tcp连接，
	//一次发送 get 一次发送post，需要通过sessioncookie关联起来
	string m_strSessionCookie;
	static recursive_mutex g_mtxGetter; //对quicktime上锁保护
	static recursive_mutex g_mtxPostter; //对quicktime上锁保护
	static unordered_map<string, weak_ptr<RtspSession> > g_mapGetter;
	static unordered_map<void *, std::shared_ptr<RtspSession> > g_mapPostter;
	static string g_serverName;

};

} /* namespace Session */
} /* namespace ZL */

#endif /* SESSION_RTSPSESSION_H_ */
