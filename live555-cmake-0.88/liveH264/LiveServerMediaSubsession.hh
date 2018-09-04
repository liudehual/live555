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
// C++ header
#ifndef _LIVE_SERVER_MEDIA_SUBSESSION_HH
#define _LIVE_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

#include "FramesManager.h"
#include "FramedLiveSource.hh"

class LiveServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
	static LiveServerMediaSubsession* createNew(UsageEnvironment& env,
			Boolean reuseFirstSource, int ringBufferSize, int framerate);

	static void setFramerate1(LiveServerMediaSubsession *session);
	int setFramerate(int framerate);

	  void checkForAuxSDPLine1();
	  void afterPlayingDummy1();

virtual void startStream(unsigned clientSessionId, void* streamToken,
			   TaskFunc* rtcpRRHandler,
			   void* rtcpRRHandlerClientData,
			   unsigned short& rtpSeqNum,
                           unsigned& rtpTimestamp,
			   ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                           void* serverRequestAlternativeByteHandlerClientData);
                           
    virtual void pauseStream(unsigned clientSessionId, void* streamToken);

	int WriteFrame(char *data, int len);

protected:
	// we're a virtual base class
	LiveServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, int ringBufferSize, int framerate);
	virtual ~LiveServerMediaSubsession();
	  void setDoneFlag() { fDoneFlag = ~0; }

protected:
	// redefined virtual functions
	virtual char const* getAuxSDPLine(RTPSink* rtpSink,
			FramedSource* inputSource);
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
			unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
			unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

protected:
	char* fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	RTPSink* fDummyRTPSink; // ditto
	FramesManager *dataManager;
	int m_ringBufferSize;
	int m_framerate;
};

#if 1
class LiveH264VideoStreamFramer: public H264VideoStreamFramer {
public:
  static LiveH264VideoStreamFramer* createNew(UsageEnvironment& env, FramedSource* inputSource,
		  double framerate, Boolean includeStartCodeInOutput = False){
	  return new LiveH264VideoStreamFramer(env, inputSource, True, includeStartCodeInOutput, framerate);
  }
  void setFramerate(int framerate);

 FramedLiveSource *fliveInputSource;
  
protected:
  virtual ~LiveH264VideoStreamFramer();
  LiveH264VideoStreamFramer(UsageEnvironment& env, FramedSource* inputSource, Boolean createParser, Boolean includeStartCodeInOutput, double framerate);

};
#endif
#endif
