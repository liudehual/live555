#ifndef _AUDIO_SERVER_MEDIA_SUBSESSION_H_
#define _AUDIO_SERVER_MEDIA_SUBSESSION_H_

#include "FramesManager.h"
#include "OnDemandServerMediaSubsession.hh"

class AudioServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	//audioType 编码方式，例如："G726-40"。这个准确的定义，请参照MediaSession::createSourceObjects中的内容
	static AudioServerMediaSubsession* createNew(UsageEnvironment& env, char const* audioType, Boolean reuseFirstSource, int bitrateKbps=64, int timeStampFrequency=16000);
	int WriteFrame(char *data, int len);

protected:
	AudioServerMediaSubsession(UsageEnvironment& env, char const* audioType, Boolean reuseFirstSource, int bitrateKbps=64, int timeStampFrequency=16000);
	virtual ~AudioServerMediaSubsession();

protected:
	virtual FramedSource * createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

public:
	FramesManager* m_fAudioReveiver;	

private:
	char m_audioType[32];
	int m_bitrateKbps;
	int m_timeStampFrequency;
};

#endif
