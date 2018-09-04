#ifndef __H264__LIVE_MEIDA_SERVER_SUB_SESSION_H__
#define __H264__LIVE_MEIDA_SERVER_SUB_SESSION_H__
#include "OnDemandServerMediaSubsession.hh"
/*
	该类创建失败
*/
class H264LiveMediaServerSubSession:public OnDemandServerMediaSubsession
{
	public:
		H264LiveMediaServerSubSession(UsageEnvironment& env, Boolean reuseFirstSource,
				portNumBits initialPortNum = 6970,
				Boolean multiplexRTCPWithRTP = False);
		virtual ~H264LiveMediaServerSubSession();
  virtual char const* sdpLines();
#if 1
  virtual void startStream(unsigned clientSessionId, void* streamToken,
			   TaskFunc* rtcpRRHandler,
			   void* rtcpRRHandlerClientData,
			   unsigned short& rtpSeqNum,
			   unsigned& rtpTimestamp,
			   ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
			   void* serverRequestAlternativeByteHandlerClientData);
  virtual void pauseStream(unsigned clientSessionId, void* streamToken);
  virtual void seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT,
			  double streamDuration, u_int64_t& numBytes);
     // This routine is used to seek by relative (i.e., NPT) time.
     // "streamDuration", if >0.0, specifies how much data to stream, past "seekNPT".  (If <=0.0, all remaining data is streamed.)
     // "numBytes" returns the size (in bytes) of the data to be streamed, or 0 if unknown or unlimited.
  virtual void seekStream(unsigned clientSessionId, void* streamToken, char*& absStart, char*& absEnd);
     // This routine is used to seek by 'absolute' time.
     // "absStart" should be a string of the form "YYYYMMDDTHHMMSSZ" or "YYYYMMDDTHHMMSS.<frac>Z".
     // "absEnd" should be either NULL (for no end time), or a string of the same form as "absStart".
     // These strings may be modified in-place, or can be reassigned to a newly-allocated value (after delete[]ing the original).
  virtual void nullSeekStream(unsigned clientSessionId, void* streamToken,
			      double streamEndTime, u_int64_t& numBytes);
     // Called whenever we're handling a "PLAY" command without a specified start time.
  virtual void setStreamScale(unsigned clientSessionId, void* streamToken, float scale);
  virtual float getCurrentNPT(void* streamToken);
  virtual FramedSource* getStreamSource(void* streamToken);
  virtual void getRTPSinkandRTCP(void* streamToken,
				 RTPSink const*& rtpSink, RTCPInstance const*& rtcp);
     // Returns pointers to the "RTPSink" and "RTCPInstance" objects for "streamToken".
     // (This can be useful if you want to get the associated 'Groupsock' objects, for example.)
     // You must not delete these objects, or start/stop playing them; instead, that is done
     // using the "startStream()" and "deleteStream()" functions.
  virtual void deleteStream(unsigned clientSessionId, void*& streamToken);

  virtual void testScaleFactor(float& scale); // sets "scale" to the actual supported scale
  virtual float duration() const;
    // returns 0 for an unbounded session (the default)
    // returns > 0 for a bounded session
  virtual void getAbsoluteTimeRange(char*& absStartTime, char*& absEndTime) const;
    // Subclasses can reimplement this iff they support seeking by 'absolute' time.

#endif

  
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
      // "estBitrate" is the stream's estimated bitrate, in kbps
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
				    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);


};
#endif
