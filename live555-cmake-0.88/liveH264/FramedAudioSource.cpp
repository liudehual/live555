
#include "FramedAudioSource.h"

#include <GroupsockHelper.hh> // for "gettimeofday()"


FramedAudioSource* FramedAudioSource::createNew(UsageEnvironment& env, FramesManager * fAudioRecevier)
{
	return  new FramedAudioSource(env, fAudioRecevier);
}
FramedAudioSource::FramedAudioSource(UsageEnvironment& env, FramesManager* fAudioReceiver) : FramedSource(env), m_fAudioReceiver(fAudioReceiver)
{
	this->m_fAudioReceiver->Start();
}

FramedAudioSource::~FramedAudioSource()
{
	m_fAudioReceiver->Stop();
}


void FramedAudioSource::doGetNextFrame()
{
	deliverFrame();
}

void FramedAudioSource::doGetNextFrame0(void* clientData) {
	FramedAudioSource *fs = (FramedAudioSource *)clientData;
	fs->doGetNextFrame();
}

void FramedAudioSource::deliverFrame()
{
	int len;
	if (!isCurrentlyAwaitingData()){
		return; 
	}
	len = m_fAudioReceiver->ReadFrameCopied(fTo, fMaxSize > 320 ? 320 : fMaxSize);
	if (len <= 0){
		fFrameSize = 0;
		int uSecsToDelay = 20 * 1000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)FramedAudioSource::doGetNextFrame0, this);
		return;
	}
	gettimeofday(&fPresentationTime, 0); 
	fFrameSize = len;
	if (fFrameSize > fMaxSize){  
		printf("error happened\n");
		fNumTruncatedBytes = fFrameSize - fMaxSize;  
		fFrameSize = fMaxSize;  
	}else{  
		fNumTruncatedBytes = 0;  
	} 
	fDurationInMicroseconds = 0; 
	FramedSource::afterGetting(this);
}
