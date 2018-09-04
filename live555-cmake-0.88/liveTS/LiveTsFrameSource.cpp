#include "LiveTsFrameSource.hh"
#include "FrameCache.hh"
LiveFrameSource::LiveFrameSource(UsageEnvironment& env,CacheManager *fm):FramedSource(env),
										frameManager(fm)
{
}
LiveFrameSource::~LiveFrameSource()
{

}
void LiveFrameSource::doGetNextFrame()
{
	if(frameManager==NULL){
		fprintf(stderr,"Not Find CacheMananger\n");
		return;
	}
	/*
		获取一帧ts数据(会包含多个ts数据包)
	*/
	unsigned char *frameBuff=NULL;
	unsigned int frameBuffLen=0;
	#if 1
	frameManager->readData(frameBuff,frameBuffLen);
	
	if(frameBuff==NULL || frameBuffLen==0){
		nextTask() = envir().taskScheduler().scheduleDelayedTask(10*1000, (TaskFunc*)getNext, this);
		return;
	}
	
	if(frameBuffLen>fMaxSize){
		memcpy(fTo,frameBuff,fMaxSize);
		fFrameSize=fMaxSize;
	}else{
		memcpy(fTo,frameBuff,frameBuffLen);
		fFrameSize=frameBuffLen;
	}

	#endif
	/*Test*/
	#if 0
	static FILE *fp=NULL;
	if(fp==NULL){
		fp=fopen("test.ts","rb");
		if(fp==NULL){
			fprintf(stderr,"Open File Error \n");
			exit(1);
		}
	}
	unsigned readChunkSize=188*7;
	while(1){
		int len=fread(fTo,1,readChunkSize,fp);
		if (len <= 0 || feof(fp)){
			fseek(fp, 0, SEEK_SET);
			printf("continue----------------------------------------------\n");
			continue;
		}
		fFrameSize=len;

		break;
	}
	#endif
	FramedSource::afterGetting(this);
}
void LiveFrameSource::getNext(void* firstArg)
{
	static int runNum=0;
	LiveFrameSource *source=(LiveFrameSource *)firstArg;
	source->doGetNextFrame();
	fprintf(stderr,"We try get next frame %d \n",runNum);
}

