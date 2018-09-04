#ifndef __LIVE_TS_FRAME_SOURCE_HH__
#define __LIVE_TS_FRAME_SOURCE_HH__
#include "FramedSource.hh"
class CacheManager;
class LiveFrameSource:public FramedSource
{
	public:
		LiveFrameSource(UsageEnvironment& env,CacheManager *fm);
		~LiveFrameSource();
		virtual void doGetNextFrame();
		static void getNext(void* firstArg);
	private:
		CacheManager *frameManager;
};
#endif
