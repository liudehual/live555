
#ifndef _FRAMED_AUDIO_SOURCE_H_
#define _FRAMED_AUDIO_SOURCE_H_

#include "FramedSource.hh"
#include "FramesManager.h"


class FramedAudioSource : public FramedSource
{
public:
	static FramedAudioSource * createNew(UsageEnvironment& env, FramesManager * fAudioRecevier);

protected:
	FramedAudioSource(UsageEnvironment& env, FramesManager* fAudioReceiver);
	virtual ~FramedAudioSource();

private:
	static void doGetNextFrame0(void* clientData);
	virtual void doGetNextFrame();

	void deliverFrame();

private:

	FramesManager* m_fAudioReceiver;	
};

#endif
