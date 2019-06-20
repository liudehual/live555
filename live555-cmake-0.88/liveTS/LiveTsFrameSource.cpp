#include "LiveTsFrameSource.hh"
#include "Cache.h"
LiveFrameSource::LiveFrameSource(UsageEnvironment& env,Cache *c):FramedSource(env),
										fCache(c)
{
}
LiveFrameSource::~LiveFrameSource()
{

}
void LiveFrameSource::doGetNextFrame()
{
	if(fCache==NULL){
		fprintf(stderr,"Not Find CacheMananger\n");
		return;
	}
	int out_len=0;
	fCache->read_data(fTo,188*5,out_len);
	//printf("[%s][%d] max size %d frame size %d \n",__FUNCTION__,__LINE__,fMaxSize,out_len);
	if(out_len==0){
		nextTask() = envir().taskScheduler().scheduleDelayedTask(10*1000, 
													(TaskFunc*)getNext, 
													this);
		return;
	}
	fFrameSize=out_len;
	FramedSource::afterGetting(this);
}
void LiveFrameSource::getNext(void* firstArg)
{
	static int runNum=0;
	LiveFrameSource *source=(LiveFrameSource *)firstArg;
	source->doGetNextFrame();
	fprintf(stderr,"We try get next frame %d \n",runNum);
}

