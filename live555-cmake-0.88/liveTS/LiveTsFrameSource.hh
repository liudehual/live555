#ifndef __LIVE_TS_FRAME_SOURCE_HH__
#define __LIVE_TS_FRAME_SOURCE_HH__
#include "FramedSource.hh"
class Cache;
class LiveFrameSource:public FramedSource
{
	public:
		LiveFrameSource(UsageEnvironment& env,Cache *c);
		~LiveFrameSource();
		virtual void doGetNextFrame();
		static void getNext(void* firstArg);
	private:
		Cache *fCache;
};
#endif
