#include "liveMedia.hh"
#include "FramedLiveSource.hh"

#include "LiveServerMediaSubsession.hh"
#include "AudioServerMediaSubsession.h"

#include "BasicUsageEnvironment.hh"
#include "Live.h"

UsageEnvironment* env;

Boolean reuseFirstSource = False;

void registerStreamHandler(RTSPServer* rtspServer, int socketNum, int resultCode, char* resultString , int chn);

#define MAX_STREAM	128
static LiveServerMediaSubsession *liveSubsession[MAX_STREAM] = {NULL};
static AudioServerMediaSubsession *audioSubsession[MAX_STREAM] = {NULL};
static RTSPServer* sRtspServer = NULL;


static LiveServerMediaSubsession *SLiveSubStream[MAX_STREAM] = {NULL};
static AudioServerMediaSubsession *SAudioSubStream[MAX_STREAM] = {NULL};


static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
		char const* streamName, int index) {
	char* url = rtspServer->rtspURL(sms);
	UsageEnvironment& env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream, from the source \""
			<< index << "\"\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

class UserEnviornment: public BasicUsageEnvironment{
public:
	UserEnviornment(TaskScheduler& taskScheduler)
	: BasicUsageEnvironment(taskScheduler) {
	}

	~UserEnviornment() {
	}

static	UserEnviornment*
	createNew(TaskScheduler& taskScheduler) {
	  return new UserEnviornment(taskScheduler);
	}
public:
	void setResultMsg(MsgString msg)
	{
		printf("%s\n", msg);
	}
};

static struct sockaddr_in sConnectAddr;

class LiveRTSPServer : public RTSPServer{
public:
	LiveRTSPServer(UsageEnvironment& env,
		     int ourSocket, Port ourPort,
		     UserAuthenticationDatabase* authDatabase,
		     unsigned reclamationTestSeconds):
		    	 RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds){

	}
                 
	static LiveRTSPServer*
	createNew(UsageEnvironment& env, Port ourPort = 554,
		       UserAuthenticationDatabase* authDatabase = NULL,
		       unsigned reclamationTestSeconds = 65*3);

	static LiveRTSPServer*
	createNew(UsageEnvironment& env,
			   UserAuthenticationDatabase* authDatabase = NULL,
			   unsigned reclamationTestSeconds = 65*3, Port ourPort=554);
    

public:
	class LiveRTSPClientSession : public RTSPClientSession
	{
	public:
		LiveRTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId)
			:RTSPClientSession(ourServer, sessionId)
		{
		}

	    ~LiveRTSPClientSession()
	    {
	    }
	};


	class LiveRTSPClientConnection : public RTSPClientConnection
	{
	public:
		LiveRTSPClientConnection(RTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
			:RTSPClientConnection(ourServer, clientSocket, clientAddr)
		{
	    	sConnectAddr = clientAddr;

		}
		~LiveRTSPClientConnection(){}
	};

	  virtual RTSPClientConnection*
	  createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);

	  virtual RTSPClientSession*
	  createNewClientSession(u_int32_t sessionId);

protected:     
      virtual ~LiveRTSPServer(){}
};

LiveRTSPServer*
LiveRTSPServer::createNew(UsageEnvironment& env, Port ourPort,
		      UserAuthenticationDatabase* authDatabase,
		      unsigned reclamationTestSeconds) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;
  return new LiveRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

LiveRTSPServer*
LiveRTSPServer::createNew(UsageEnvironment& env,
		      UserAuthenticationDatabase* authDatabase,
		      unsigned reclamationTestSeconds, Port ourPort) {
  return new LiveRTSPServer(env, -1, ourPort, authDatabase, reclamationTestSeconds);
}

RTSPServer::RTSPClientConnection*
LiveRTSPServer::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr) {
  return new LiveRTSPClientConnection(*this, clientSocket, clientAddr);
}

RTSPServer::RTSPClientSession*
LiveRTSPServer::createNewClientSession(u_int32_t sessionId) {
  return new LiveRTSPClientSession(*this, sessionId);
}

static UserAuthenticationDatabase *sAuthDB = NULL;

//准备RTSP服务
//param nameFmt 类似live%d.264
//cnt 提供的视频的路数。
void liveRtspStartEx(char *nameFmt, unsigned short port, int *bufferSizeList, int *frameList, int cnt, char *audioType, int timeStampFrequency)
{
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* environment = BasicUsageEnvironment::createNew(*scheduler);

    char const* descriptionString = "Session streamed by \"rtspServer\"";

    int i;

	UserAuthenticationDatabase* authDB = NULL;

    fprintf(stderr,"Starting...\n");

    RTSPServer* rtspServer = LiveRTSPServer::createNew(*environment, port, authDB);
	if (rtspServer == NULL){
	    *environment << "Failed to create RTSP server: " << environment->getResultMsg() << "\n";
	    exit(1);
	}

    
	for (i = 0; i < cnt; i++){
		char streamName[256];
		if (cnt > MAX_STREAM){
			break;
		}
        
		sprintf(streamName, nameFmt, i);
		ServerMediaSession* sms = ServerMediaSession::createNew(*environment,
				streamName, streamName, descriptionString);
		Boolean ret;
#if 1
		SLiveSubStream[i] = LiveServerMediaSubsession::createNew(*environment, 1, bufferSizeList[i], frameList[i]);
		ret = sms->addSubsession(SLiveSubStream[i]);
		if (!ret){
			printf("addSubsession Failed\n");
		}
#endif

		if (audioType && timeStampFrequency != 0){
			SAudioSubStream[i] = AudioServerMediaSubsession::createNew(*environment, audioType, reuseFirstSource, 40, timeStampFrequency);
			ret = sms->addSubsession(SAudioSubStream[i]);
			if (!ret){
				fprintf(stderr,"Second addSubsession Failed\n");
			}
		}
		rtspServer->addServerMediaSession(sms);

		announceStream(rtspServer, sms, streamName, i);
	}
	environment->taskScheduler().doEventLoop();
}



//准备RTSP服务
//param nameFmt 类似live%d.264
//cnt 提供的视频的路数。
void liveRtspStart(char *nameFmt, unsigned short port, int *bufferSizeList, int *frameLisit, int cnt) {
	liveRtspStartEx(nameFmt, port, bufferSizeList, frameLisit, cnt, NULL, 8000);
}


//写入数据
int liveRtspWrite(int index, char *data, int len) 
{
	if (liveSubsession[index]){
		return liveSubsession[index]->WriteFrame(data, len);
	}
	return -1;
}

int liveRtspWriteAudio(int index, char *data, int len)
{
	if (audioSubsession[index])
		return audioSubsession[index]->WriteFrame(data, len);
	return -1;
}


//写入数据
int liveRtspWriteEx(int index, char *data, int len) 
{
	if (SLiveSubStream[index]){
		return SLiveSubStream[index]->WriteFrame(data, len);
	}
	return -1;
}

int liveRtspSetFramerate(int index, int framerate) 
{
	if (liveSubsession[index]){
		return liveSubsession[index]->setFramerate(framerate);
	}
	return -1;
}

