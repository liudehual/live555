#ifndef __H264_LIVE_MEDIA_SERVER_SUB_SESSION_H__
#define __H264_LIVE_MEDIA_SERVER_SUB_SESSION_H__
#include "FramedSource.hh"
/*
	���๦����ʧ�ܵ�
	��Ҫʹ�� 
*/
class H264LiveMediaServerFrameSource:public FramedSource
{
	public:
		H264LiveMediaServerFrameSource(UsageEnvironment& env);
		~H264LiveMediaServerFrameSource();
		virtual void doGetNextFrame();
		
};
#endif
