/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2012, Live Networks, Inc.  All rights reserved
// A demo application, showing how to create and run a RTSP client (that can potentially receive multiple streams concurrently).
//
// NOTE: This code - although it builds a running application - is intended only to illustrate how to develop your own RTSP
// client application.  For a full-featured RTSP client application - with much more functionality, and many options - see
// "openRTSP": http://www.live555.com/openRTSP/

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "LiveClient.h"

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
RTSPClient* openURL(UsageEnvironment& env, char const* progName, char const* rtspURL,
		const char *user,
		const char *passwd,
		afterPlayingFunc_t afterPlaying,
		afterGettingFrame_t afterGetting,
		void *param);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName) {
  env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
  env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

class StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;

  afterPlayingFunc_t *afterPlayingPtr;
  afterGettingFrame_t *afterGettingFramePtr;
  void *userParam;
  int maxVideoStreamSize;	//最大帧的帧大小
  char exitFlag;
};
class ourRTSPClient: public RTSPClient {
public:
  static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
  ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~ourRTSPClient();

public:
  StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
  static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      StreamClientState &clientState,
			      char const* streamId = NULL
			      ); // identifies the stream itself (optional)

private:
  DummySink(UsageEnvironment& env, MediaSubsession& subsession, StreamClientState &clientState, char const* streamId);
    // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

public:
	MediaSubsession& fSubsession;

private:
  u_int8_t* fReceiveBuffer;
  int fMaxReceiveLen;
  char* fStreamId;
  StreamClientState& clientState;
};

#define RTSP_CLIENT_VERBOSITY_LEVEL 0 // by default, print verbose output from each "RTSPClient"

typedef struct{
	void *param;
	int linkid;
	char token[64];
}list_info_t;
#define MAX_LIST_CNT 128
static list_info_t sClientList[MAX_LIST_CNT];
static int sClientCnt = 0;
static int __list_added(void *param, const m_streamID_t *token)
{
	sClientList[sClientCnt].param = param;
	sClientList[sClientCnt].linkid = token->m_LinkID;
	strncpy(sClientList[sClientCnt].token, token->m_streamID, sizeof(sClientList[sClientCnt].token));
	sClientCnt++;

	return 0;
}

static int __list_delete(const m_streamID_t *token)
{
	int i;
	for (i=0;i<sClientCnt;i++)
	{
		if (strcmp(token->m_streamID, sClientList[i].token) == 0)
		{
			if (sClientCnt-1 != i)
				memcpy(&sClientList[i], &sClientList[i+1], (sClientCnt-i-1)*sizeof(list_info_t));
			sClientCnt--;
			return 0;
		}
	}

	printf("Failed find token to delete");
	return -1;
}

static list_info_t *__list_get(const m_streamID_t *token)
{
	int i;
	for (i=0;i<sClientCnt;i++)
	{
		if (strcmp(token->m_streamID, sClientList[i].token) == 0 ||token->m_LinkID == sClientList[i].linkid)
		{
			return &sClientList[i];
		}
	}

	printf("Failed find token to get\n");

	return NULL;
}

/**
 * 打开一路码流
 * 注意：本函数不返回，会阻塞，直到调用jvlive_client_stop
 *
 *@param streamID 流名称，唯一标识打开的流
 *@param url 要打开的数据流的地址，类似：rtsp://192.168.11.192:8554/live0.264
 *@param maxVideoStreamSize 视频最大帧的帧大小
 *@param user 用户名，无则为NULL
 *@param passwd 密码，无则为NULL
 *@param afterPlaying 主控结束，或者其它原因导致停止时的回调
 *@param afterGetting 收到数据后的回调函数
 *@param param 用户数据，会在afterPlaying和afterGetting时返回
 */
 int jvlive_client_run(
 		const m_streamID_t *streamID,
		const char *url,
		int maxVideoStreamSize,
		const char *user,
		const char *passwd,
		afterPlayingFunc_t afterPlaying,
		afterGettingFrame_t afterGetting,
		void *param)
{

	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	  RTSPClient* rtspClient = ourRTSPClient::createNew(*env, url, RTSP_CLIENT_VERBOSITY_LEVEL, (const char *)streamID->acManufacture);
	  if (rtspClient == NULL) {
	    *env << "Failed to create a RTSP client for URL \"" << url << "\": " << env->getResultMsg() << "\n";
	    return -1;
	  }

	  ((ourRTSPClient *)rtspClient)->scs.afterGettingFramePtr = afterGetting;
	  ((ourRTSPClient *)rtspClient)->scs.afterPlayingPtr = afterPlaying;
	  ((ourRTSPClient *)rtspClient)->scs.userParam = param;
	  ((ourRTSPClient *)rtspClient)->scs.maxVideoStreamSize = maxVideoStreamSize;
	  char *exitFlag = &((ourRTSPClient *)rtspClient)->scs.exitFlag;
	  *exitFlag = 0;
	  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	  
	  rtspClient->sendDescribeCommand(continueAfterDESCRIBE, new Authenticator(user, passwd));

	  __list_added((void *)rtspClient,streamID);
	// All subsequent activity takes place within the event loop:
	env->taskScheduler().doEventLoop(exitFlag);
	// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.


	// If you choose to continue the application past this point (i.e., if you comment out the "return 0;" statement above),
	// and if you don't intend to do anything more with the "TaskScheduler" and "UsageEnvironment" objects,
	// then you can also reclaim the (small) memory used by these objects by uncommenting the following code:
	shutdownStream(rtspClient, 0);
	env->reclaim(); env = NULL;
	delete scheduler; scheduler = NULL;
	return 0;
}

/**
 * 关闭之前打开的码流
 */
int jvlive_client_stop(const m_streamID_t *streamID)
{
	list_info_t *info;
	int i;
	for (i=0;i<10;i++)
	{
		info = __list_get(streamID);
		if (!info)
		{

			{
			#if defined(__WIN32__) || defined(_WIN32)
				Sleep(300);
			#else
				usleep(300*1000);
			#endif
			}
			continue;
		}
		break;
	}
	if (!info)
	{
		return -1;
	}
	ourRTSPClient *rtspClient = (ourRTSPClient *)info->param;
	rtspClient->scs.exitFlag = 1;
	__list_delete(streamID);
	return 0;
}

static jvlive_err_e __rtspErrorcode2jverr(int rtspErrcode)
{
	switch(rtspErrcode)
	{
	default:
		return JVLIVE_ERR_UNKNOWN;
		break;
	case 200:
		return JVLIVE_ERR_OK;
	case 110:
		return JVLIVE_ERR_TIMEOUT;
	case 400:
		return JVLIVE_ERR_BAD_REQUEST;
	case 401:
		return JVLIVE_ERR_WRONG_PASSWD;
	case 404:
		return JVLIVE_ERR_NOT_FOUND;
	}
}


// Implementation of the RTSP 'response handlers':


static char String[512]={0};
static int get_string_OK = 0;
char *GetParameter_String()
{
	//get_string_OK =0 ;
	int i=0;
	for (i=0;i<20;i++)
	{
		if(!get_string_OK)
		{
			#if defined(__WIN32__) || defined(_WIN32)
			Sleep(100);
			#else
			usleep(100*1000);
			#endif
		}
		else
		{
			break;
		}
	}
	if(get_string_OK)
	{
		get_string_OK=0;
		return String;
	}
	return NULL;
}



void continueAfterGET_PARAMETER(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); 
    //StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; 

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      break;
    }

    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
	int Len = 0;
	{
		Len = strlen(sdpDescription);
		
		memset(String,0,sizeof(char)*Len +1);
		memcpy(String,sdpDescription,Len);
		get_string_OK =1;
    }
	//strstr(sdpDescription,)
    delete[] sdpDescription; 

  } while (0);

}


int GetParameter_session(
		const m_streamID_t *streamID,
		char const* parameterName,
		const char *user,
		const char *passwd) 

{
	list_info_t *info;
	int i;
	for (i=0;i<10;i++)
	{
		info = __list_get(streamID);
		if (!info)
		{

			{
		#if defined(__WIN32__) || defined(_WIN32)
				Sleep(300);
		#else
				usleep(300*1000);
		#endif
			}
			continue;
		}
		break;
	}
	if (!info)
	{
		return -1;
	}
	ourRTSPClient *rtspClient = (ourRTSPClient *)info->param;

  //UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
  rtspClient->sendGetParameterCommand(*scs.session,continueAfterGET_PARAMETER,parameterName,new Authenticator(user, passwd));
  return 1;
}

void continueAfterSET_PARAMETER(RTSPClient* rtspClient, int resultCode, char* resultString) {

}

int SetParameter_session(
		const m_streamID_t *streamID,
		char const* parameterValue,
		const char *user,
		const char *passwd) 
{
	list_info_t *info;
	int i;
	for (i=0;i<10;i++)
	{
		info = __list_get(streamID);
		if (!info)
		{

			{
		#if defined(__WIN32__) || defined(_WIN32)
				Sleep(300);
		#else
				usleep(300*1000);
		#endif
			}
			continue;
		}
		break;
	}
	if (!info)
	{
		return -1;
	}
	ourRTSPClient *rtspClient = (ourRTSPClient *)info->param;

  //UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  scs.duration = scs.session->playEndTime() - scs.session->playStartTime();

  
  rtspClient->sendSetParameterCommand(*scs.session,continueAfterSET_PARAMETER,NULL,parameterValue,new Authenticator(user, passwd));
  return 0;
}


void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      if (scs.afterPlayingPtr)
      {
    	  scs.afterPlayingPtr(scs.userParam, __rtspErrorcode2jverr(resultCode), resultString);
      }
      break;
    }

    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
//  shutdownStream(rtspClient);
}

void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession
	  << "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, false, false);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
  rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      if (scs.afterPlayingPtr)
      {
    	  scs.afterPlayingPtr(scs.userParam, __rtspErrorcode2jverr(resultCode), resultString);
      }
     break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession
	<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, scs, rtspClient->url());
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      if (scs.afterPlayingPtr)
      {
    	  scs.afterPlayingPtr(scs.userParam, __rtspErrorcode2jverr(resultCode), resultString);
      }
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
//  shutdownStream(rtspClient);
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  ourRTSPClient *our = (ourRTSPClient *)rtspClient;
  if (our->scs.afterPlayingPtr)
  {
	  our->scs.afterPlayingPtr(our->scs.userParam, JVLIVE_ERR_OVER, "");
  }
  // All subsessions' streams have now been closed, so shutdown the client:
//  shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
//  shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.

//  if (--rtspClientCount == 0) {
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you might want to comment this out,
    // and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
//    exit(exitCode);
//  }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
}

ourRTSPClient::~ourRTSPClient() {
}


// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0),
    afterPlayingPtr(NULL),
    afterGettingFramePtr(NULL),
    userParam(NULL),
    maxVideoStreamSize(0),
    exitFlag(0)
    {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 512*1024

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, StreamClientState &clientState, char const* streamId) {
  return new DummySink(env, subsession, clientState, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, StreamClientState &clientState, char const* streamId)
  : MediaSink(env),
    fSubsession(subsession),
    clientState(clientState){
	/*
	*******************************
	lfx @ 2013-6-26
	for video H264
	(gdb) p scs.subsession.fSavedSDPLines
	$1 = 0x881b3c8 "m=video 0 RTP/AVP 96\r\nc=IN IP4 0.0.0.0\r\nb=AS:3072\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 packetization-mode=1;profile-level-id=42001F;sprop-parameter-sets=Z0IAH5WoFAFuQA==,aM48gA==\r\na=control:track1\r\n"
	(gdb) p scs.subsession.fMediumName
	$2 = 0x881b4f0 "video"
	(gdb) p scs.subsession.fCodecName
	$3 = 0x881b240 "H264"
	(gdb) p scs.subsession.fProtocolName
	$4 = 0x881b230 "RTP"


	*******************************
	for audio PCMU

	(gdb) p scs.subsession.fMediumName
	$10 = 0x881b4e8 "audio"
	(gdb) p scs.subsession.fCodecName
	$11 = 0x881a870 "PCMU"
	(gdb) p scs.subsession.fProtocolName
	$12 = 0x881b190 "RTP"
	(gdb) p scs.subsession.fProtocolName
	 */
  fStreamId = strDup(streamId);
  printf("mediaName: %s\n", subsession.mediumName());
  if (strcmp(subsession.mediumName(), "video") == 0)
	  fMaxReceiveLen = clientState.maxVideoStreamSize;
  else
	  fMaxReceiveLen = 10*1024;
  fReceiveBuffer = new u_int8_t[fMaxReceiveLen];
}

DummySink::~DummySink() {
  delete[] fReceiveBuffer;
  delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 0

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
#if DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (unsigned)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
  unsigned nowUsec;
  static unsigned oldUsec;
  nowUsec = presentationTime.tv_sec * 1000*1000 + presentationTime.tv_usec;
  envir() << " duration: " << (nowUsec - oldUsec);
  oldUsec = nowUsec;
  envir() << "\n";
#endif
//  printf("streamid: %s\n", fStreamId);
//  printf("sink this: %08x", this);
  if (clientState.afterGettingFramePtr)
  {
	  clientState.afterGettingFramePtr(clientState.userParam, this, fReceiveBuffer, frameSize, presentationTime, 0);
  }

  // Then continue, to request the next frame of data:
  continuePlaying();
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, fMaxReceiveLen,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}

/**
 * 获取RTSP播放的某码 流具体信息
 *
 *@param channelHandle #afterGettingFrame_t 的参数之一
 *@param info 获取到的信息
 */
int jvlive_client_get_channel_info(void *channelHandle, jvliveChannelInfo_t *info)
{
	DummySink *sink = (DummySink *)channelHandle;
	strncpy(info->mediaName, sink->fSubsession.mediumName(), sizeof(info->mediaName));
	strncpy(info->codecName, sink->fSubsession.codecName(), sizeof(info->codecName));
	strncpy(info->protocolName, sink->fSubsession.protocolName(), sizeof(info->protocolName));
	return 0;
}
