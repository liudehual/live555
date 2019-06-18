#include "LiveTs.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "LiveTsServerMediaSubSession.hh"
#include "ServerMediaSession.hh"
#include "FrameCache.hh"
#define MAX_LIVE_SUB_SESSION_NUM 256

Boolean reuseFirstSource = False;


LiveTsServerSubSession * liveSubSession[MAX_LIVE_SUB_SESSION_NUM]={NULL};
RTSPServer* rtspServer=NULL;
UsageEnvironment* env;

int startRtspServer(unsigned short port)
{
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create the RTSP server:
  rtspServer = RTSPServer::createNew(*env, port, NULL);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}
int setChannel(unsigned int chNum)
{
	if(chNum>MAX_LIVE_SUB_SESSION_NUM || chNum<=0){
		fprintf(stderr," chNum > %d || chNum <=0 \n",MAX_LIVE_SUB_SESSION_NUM);
		return 0;
	}
	if(rtspServer==NULL){
		fprintf(stderr,"Not Create RtspServer \n");
		return 0;
	}
	for(int i=0;i<chNum;++i){
    	char const* streamName = "live%d.ts";
    	
		char streamNameBuf[16]={'\0'};
		sprintf(streamNameBuf,streamName,i);
		fprintf(stderr,"%s\n",streamNameBuf);
		LiveTsServerSubSession  *tsSubSession=LiveTsServerSubSession
    								::createNew(*env,NULL,reuseFirstSource);
		
		liveSubSession[i]=tsSubSession;
		
    	ServerMediaSession* sms
      		= ServerMediaSession::createNew(*env, streamNameBuf, streamNameBuf);
    	sms->addSubsession(tsSubSession);
    	rtspServer->addServerMediaSession(sms);
	}
}

int writeData(unsigned char *buf,unsigned int bufLen,int channel)
{
	if(channel>MAX_LIVE_SUB_SESSION_NUM || liveSubSession[channel]==NULL){
		fprintf(stderr,"SomeThing is Error While WriteData \n");
		return 0;
	}	
	liveSubSession[channel]->getCacheManager()->writeData(buf,bufLen);

	return 1;
}
