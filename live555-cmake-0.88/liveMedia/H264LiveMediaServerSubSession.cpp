#include "H264LiveMediaServerSubSession.hh"
H264LiveMediaServerSubSession::H264LiveMediaServerSubSession(UsageEnvironment& env, Boolean reuseFirstSource,
                                                             portNumBits initialPortNum ,
                                                             Boolean multiplexRTCPWithRTP):OnDemandServerMediaSubsession(env,reuseFirstSource)
{

}
H264LiveMediaServerSubSession::~H264LiveMediaServerSubSession()
{

}
char const* H264LiveMediaServerSubSession::sdpLines()
{
	return NULL;
}

FramedSource* H264LiveMediaServerSubSession::createNewStreamSource(unsigned clientSessionId,
                                                                   unsigned& estBitrate)
{
	return NULL;
}
// "estBitrate" is the stream's estimated bitrate, in kbps
RTPSink* H264LiveMediaServerSubSession::createNewRTPSink(Groupsock* rtpGroupsock,
                                                         unsigned char rtpPayloadTypeIfDynamic,
                                                         FramedSource* inputSource)
{
	return NULL;
}
#if 1
void H264LiveMediaServerSubSession::startStream(unsigned clientSessionId, void* streamToken,
                                                TaskFunc* rtcpRRHandler,
                                                void* rtcpRRHandlerClientData,
                                                unsigned short& rtpSeqNum,
                                                unsigned& rtpTimestamp,
                                                ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                                                void* serverRequestAlternativeByteHandlerClientData)
{

}
void H264LiveMediaServerSubSession::pauseStream(unsigned clientSessionId, void* streamToken)
{

}
void H264LiveMediaServerSubSession::seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT,
                                               double streamDuration, u_int64_t& numBytes)
{

}
// This routine is used to seek by relative (i.e., NPT) time.
// "streamDuration", if >0.0, specifies how much data to stream, past "seekNPT".  (If <=0.0, all remaining data is streamed.)
// "numBytes" returns the size (in bytes) of the data to be streamed, or 0 if unknown or unlimited.
void H264LiveMediaServerSubSession::seekStream(unsigned clientSessionId, void* streamToken, char*& absStart, char*& absEnd)
{

}
// This routine is used to seek by 'absolute' time.
// "absStart" should be a string of the form "YYYYMMDDTHHMMSSZ" or "YYYYMMDDTHHMMSS.<frac>Z".
// "absEnd" should be either NULL (for no end time), or a string of the same form as "absStart".
// These strings may be modified in-place, or can be reassigned to a newly-allocated value (after delete[]ing the original).
void H264LiveMediaServerSubSession::nullSeekStream(unsigned clientSessionId, void* streamToken,
                                                   double streamEndTime, u_int64_t& numBytes)
{

}
// Called whenever we're handling a "PLAY" command without a specified start time.
void H264LiveMediaServerSubSession::setStreamScale(unsigned clientSessionId, void* streamToken, float scale)
{

}
float H264LiveMediaServerSubSession::getCurrentNPT(void* streamToken)
{
	return 0.0;
}
FramedSource* H264LiveMediaServerSubSession::getStreamSource(void* streamToken)
{
	return NULL;
}
void H264LiveMediaServerSubSession::getRTPSinkandRTCP(void* streamToken,
                                                      RTPSink const*& rtpSink, RTCPInstance const*& rtcp)
{

}
// Returns pointers to the "RTPSink" and "RTCPInstance" objects for "streamToken".
// (This can be useful if you want to get the associated 'Groupsock' objects, for example.)
// You must not delete these objects, or start/stop playing them; instead, that is done
// using the "startStream()" and "deleteStream()" functions.
void H264LiveMediaServerSubSession::deleteStream(unsigned clientSessionId, void*& streamToken)
{

}

void H264LiveMediaServerSubSession::testScaleFactor(float& scale) // sets "scale" to the actual supported scale
{

}
float H264LiveMediaServerSubSession::duration() const
{
	return 0;
}
// returns 0 for an unbounded session (the default)
// returns > 0 for a bounded session
void H264LiveMediaServerSubSession::getAbsoluteTimeRange(char*& absStartTime, char*& absEndTime) const
{

}
// Subclasses can reimplement this iff they support seeking by 'absolute' time.

#endif

