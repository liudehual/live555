#include "liveThread.hh"
LiveThread::LiveThread(unsigned short port)
{
	scheduler=BasicTaskScheduler::createNew();
	env=BasicUsageEnvironment::createNew(*scheduler);
	if(port<0){
		mPort=554;
	}else{
		mPort=port;	
	}
	rtspServer = DynamicRTSPServer::createNew(*env,mPort,NULL);
	if(rtspServer!=NULL){
		char* urlPrefix = rtspServer->rtspURLPrefix();
		fprintf(stderr,"%s\n",urlPrefix);
	}else{
		fprintf(stderr,"Create RTSPServer Error errno %d\n",errno);
	}
}

LiveThread::~LiveThread()
{
}
void * LiveThread::Run(void *arg)
{
	if(arg==NULL) return NULL;
	LiveThread *live=(LiveThread *)arg;
	live->getEnv()->taskScheduler().doEventLoop();
	return NULL;
}
