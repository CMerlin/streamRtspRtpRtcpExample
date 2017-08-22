/*
 * MediaSource.h
 *
 *  Created on: 2016年8月10日
 *      Author: xzl
 */

#include "RtspMediaSource.h"
#include "MediaFile/MediaReader.h"

using namespace ZL::MediaFile;

namespace ZL {
namespace Rtsp {

recursive_mutex RtspMediaSource::g_mtxMediaSrc;
unordered_map<string, unordered_map<string, weak_ptr<RtspMediaSource> > > RtspMediaSource::g_mapMediaSrc;

/**************************************************************
* decription:查找视频文件是否存在
* inParam strApp:视频文件所在目录
* inParam strID:视频文件的名称
* Return：视频文件相关的信息
***************************************************************/
RtspMediaSource::Ptr RtspMediaSource::find(const string &strApp, const string &strId,bool bMake) {
	//查找某一媒体源，找到后返回
	lock_guard<recursive_mutex> lock(g_mtxMediaSrc);
	auto itApp = g_mapMediaSrc.find(strApp);
#if 0 /*merlin 预览map中的所有成员*/
	int i = 0;
	typedef unordered_map<string, unordered_map<string, weak_ptr<RtspMediaSource> > > unMap;
	 for(unMap::iterator iter=g_mapMediaSrc.begin();iter!=g_mapMediaSrc.end();iter++ ){
	 	i++;
        cout<<"key="<<iter->first<<" and value="<<i<<endl;
    }
#endif
	if (itApp == g_mapMediaSrc.end()) {
		return bMake ? MediaReader::onMakeRtsp(strApp, strId) : nullptr;
	}
	auto itId = itApp->second.find(strId);
	if (itId == itApp->second.end()) {
		return bMake ? MediaReader::onMakeRtsp(strApp, strId) : nullptr;
	}
	auto ret = itId->second.lock();
	if(ret){
		return ret;
	}
	itApp->second.erase(itId);
	if (itApp->second.size() == 0) {
		g_mapMediaSrc.erase(itApp);
	}
	return bMake ? MediaReader::onMakeRtsp(strApp, strId) : nullptr;
}

} /* namespace Rtsp */
} /* namespace ZL */
