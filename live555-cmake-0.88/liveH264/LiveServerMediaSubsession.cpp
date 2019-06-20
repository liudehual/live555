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
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a file.
// Implementation
#include "liveMedia.hh"
//#include "FramedLiveSource.hh"

#include "LiveServerMediaSubsession.hh"


LiveServerMediaSubsession*
LiveServerMediaSubsession::createNew(UsageEnvironment& env,
					      Boolean reuseFirstSource,
					      int ringBufferSize,
					      int framerate) {
  return new LiveServerMediaSubsession(env, reuseFirstSource, ringBufferSize, framerate);
}

LiveServerMediaSubsession::LiveServerMediaSubsession(UsageEnvironment& env,
								       Boolean reuseFirstSource, int ringBufferSize, int framerate)
  : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL),
    dataManager(NULL), m_ringBufferSize(ringBufferSize),
    m_framerate(framerate)
{
}

LiveServerMediaSubsession::~LiveServerMediaSubsession() {
  delete[] fAuxSDPLine;
  delete dataManager;
  dataManager = NULL;
}

static void afterPlayingDummy(void* clientData) {
	LiveServerMediaSubsession* subsess = (LiveServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}

void LiveServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
	LiveServerMediaSubsession* subsess = (LiveServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void LiveServerMediaSubsession::checkForAuxSDPLine1() {
  char const* dasl;

  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
    fAuxSDPLine = strDup(dasl);
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* LiveServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
    // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}
using namespace std;
#include <map>
typedef map<LiveH264VideoStreamFramer *, LiveServerMediaSubsession *> liveMap_t;
static liveMap_t liveMap;


FramedSource* LiveServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) 
{
    FramedSource *framedSource;
    FramedSource *ret;
    
    estBitrate = 3072; // kbps, estimate

    OutPacketBuffer::maxSize = 2*1024*1024; // bytes

    // Create the video source:
    if (!dataManager)
    {
        dataManager = new FramesManager(m_ringBufferSize);
    }
    
    framedSource = new FramedLiveSource(envir(), dataManager);

    if (framedSource == NULL) return NULL;

    
    //framedSource->fclientsessionID = clientSessionId;
    //printf("framedSource->fclientsessionID:%d\n", framedSource->fclientsessionID);
    

    ret = LiveH264VideoStreamFramer::createNew(envir(), framedSource, (double)m_framerate);
    
    // Create a framer for the Video Elementary Stream:
    //printf("H264VideoStreamFramer: %p, framerate: %f\n", ret, (double)m_framerate);
    
    liveMap[(LiveH264VideoStreamFramer *)ret] = this;

    //  m_subSession = ret;

    return ret;
}

void LiveServerMediaSubsession::setFramerate1(LiveServerMediaSubsession *session)
{
	liveMap_t::iterator it = liveMap.begin();
	int cnt=0;
	while(it != liveMap.end())
	{
		cnt++;
		if (it->second == session)
		{
			it->first->setFramerate(session->m_framerate);
			return ;
		}
		it++;
	}
	//printf("qndy source cnt: %d\n", cnt);
}

void LiveServerMediaSubsession::startStream(unsigned clientSessionId,
                        void* streamToken,
                        TaskFunc* rtcpRRHandler,
                        void* rtcpRRHandlerClientData,
                        unsigned short& rtpSeqNum,
                        unsigned& rtpTimestamp,
                        ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                        void* serverRequestAlternativeByteHandlerClientData)
{
    StreamState* streamState = (StreamState*)streamToken;
    Destinations* destinations
                    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
    if (streamState != NULL)
    {

        LiveH264VideoStreamFramer* source = (LiveH264VideoStreamFramer*)streamState->GetfMediaSource();        
        source->fliveInputSource->dataManager->Start();

        streamState->startPlaying(destinations,clientSessionId,
                                    rtcpRRHandler, rtcpRRHandlerClientData,
                                serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
        
        RTPSink* rtpSink = streamState->rtpSink(); // alias

        if (rtpSink != NULL) 
        {
            rtpSeqNum = rtpSink->currentSeqNo();
            rtpTimestamp = rtpSink->presetNextTimestamp();
        }
    }
}

void LiveServerMediaSubsession::pauseStream(unsigned /*clientSessionId*/,
                        void* streamToken) {
    // Pausing isn't allowed if multiple clients are receiving data from
    // the same source:
    if (fReuseFirstSource) return;

    StreamState* streamState = (StreamState*)streamToken;
    if (streamState != NULL) 
    {
        streamState->pause();

        LiveH264VideoStreamFramer* source = (LiveH264VideoStreamFramer*)streamState->GetfMediaSource();

        source->flushInput();
        source->fliveInputSource->dataManager->Stop();
    }
}


int LiveServerMediaSubsession::setFramerate(int framerate) {
	m_framerate = framerate;
//	return 0;

	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
				(TaskFunc*)LiveServerMediaSubsession::setFramerate1, this);
	return 0;
}

RTPSink* LiveServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
	RTPSink *sink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  return sink;
}

//extern int print_flag ;
//extern FILE *print_fp;

int LiveServerMediaSubsession::WriteFrame(char *data, int len)
{
////	printf("frameSource ==: %p\n", framedSource);

//	if (0 == access("/dev/print", F_OK))
//	{
//		if (print_fp == NULL)
//			print_fp = fopen("/tmp/prt.txt", "wb");
//		print_flag = 1;
//	}
//	else
//	{
//		print_flag = 0;
//		if (print_fp)
//		{
//			fclose(print_fp);
//			print_fp = NULL;
//		}
//	}

	if (dataManager){
		return dataManager->WriteFrame((unsigned char *)data, len);
	}
	return -1;

#if 0 //添加头，在H264解析时，可以不去挨个解析
//	static unsigned char lenFrame[9] = {0,0,0,1,20};
//	char *p;
//	int nlen;
//	int hlen;
//
	if (dataManager)
	{
		do
		{
			unsigned char nal_unit_type = data[4] & 0x1F;
//			printf("nal_unit_type: %d, %x,%x,%x,%x,%x\n", nal_unit_type, data[0],data[1], data[2], data[3], data[4]);
			switch (nal_unit_type)
			{
			default:
			case 6:
			case 7:
			case 8:
//				return dataManager->WriteFrame((unsigned char *)data, len);



				p = data+4;
				while(1)
				{
					unsigned int temp;
					if (p - data >= len)
					{
						if (len + 9 > dataManager->GetFree())
							return -1;
						nlen = htonl(len);
//						printf("===>data hlen: %d, nal: 0x%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", len, data[4], data[5], data[6], data[7], data[8], data[9]);
//						printf("data hlen: %d\n", len);
						memcpy(&lenFrame[5], &nlen, 4);
						dataManager->WriteFrame(lenFrame, 9);
						return dataManager->WriteFrame((unsigned char *)data, len);
					}
					temp = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
					if (temp == 1)
					{
						hlen = p - data;
						nlen = htonl (hlen);
						memcpy(&lenFrame[5], &nlen, 4);
//						printf("hlen: %d\n", hlen);
						if (hlen + 9 > dataManager->GetFree())
							return -1;
						dataManager->WriteFrame(lenFrame, 9);
						dataManager->WriteFrame((unsigned char *)data, hlen);
						len -= hlen;
						data = p;
//						p += 4;
						break;
					}
					else if (temp > 1)
					{
//						printf("0x%08x,", temp);
						p += 1;
					}
					else
					{
//						printf("KK0x%08x,", temp);
						p += 4;
					}
				}
				break;
			case 5:
			case 1:
				if (len + 9 > dataManager->GetFree())
					return -1;
				nlen = htonl(len);
				if (0)
				{
					int i;
					char *pp = &data[4];
					printf("h264...");
					for (i=0;i<32;i++)
						printf("%02x,", pp[i]);
					printf("\n");
				}
//				printf("data hlen: %d, nal: 0x%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", len, data[4], data[5], data[6], data[7], data[8], data[9]);
				memcpy(&lenFrame[5], &nlen, 4);
				dataManager->WriteFrame(lenFrame, 9);
				return dataManager->WriteFrame((unsigned char *)data, len);
				break;
			}
		}while(p - data <= len);
	}
	else
		return -1;
#endif
}


void LiveH264VideoStreamFramer::setFramerate(int framerate){
	  fFrameRate = framerate;
	  printf("setted framerate: %f\n", fFrameRate);
}

LiveH264VideoStreamFramer::~LiveH264VideoStreamFramer()
{
	liveMap.erase(this);
}

LiveH264VideoStreamFramer::LiveH264VideoStreamFramer(UsageEnvironment& env, FramedSource* inputSource, Boolean createParser, Boolean includeStartCodeInOutput, double framerate)
	  : H264VideoStreamFramer(env, inputSource, createParser, includeStartCodeInOutput)
{
	fFrameRate = framerate; // We assume a frame rate of 25 fps, unless we learn otherwise (from parsing a Sequence Parameter Set NAL unit)
    fliveInputSource = (FramedLiveSource *)inputSource;
}

