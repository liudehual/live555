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
#include "FramesManager.h"
#include "FramedLiveSource.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"


FramedLiveSource::FramedLiveSource(UsageEnvironment& env, FramesManager *dataManager) :
		FramedSource(env)
{
	this->dataManager = dataManager;
	this->dataManager->Start();
}


FramedLiveSource::~FramedLiveSource() {
	dataManager->Stop();
}


unsigned FramedLiveSource::maxFrameSize() const {
	return 250 * 1024;
}

void FramedLiveSource::doGetNextFrame0(void* clientData){
	((FramedLiveSource *)clientData)->doGetNextFrame();
}


void FramedLiveSource::doGetNextFrame() {
	int len;
	unsigned char *buffer;
	while(1)
	{
		len = dataManager->ReadFrame(&buffer, fMaxSize);
		if (len <= 0)
		{
			nextTask() = envir().taskScheduler().scheduleDelayedTask(30*1000,
						(TaskFunc*)FramedLiveSource::doGetNextFrame0, this);
			return ;
		}
		else
			break;
	}

#if DEBUG_SHOWCHN
    printf("FramedLiveSource->doGetNextFrame len:%d fMaxSize:%d \n", len, fMaxSize);
#endif
	
	if (fMaxSize < len)
	{
		printf("fMaxSize too Little: %d < %d\n", fMaxSize, len);
		return ;
	}
	memcpy(fTo,buffer,len);
	{
		gettimeofday(&fPresentationTime, NULL);
	}
	fFrameSize = len;
	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
				(TaskFunc*)FramedSource::afterGetting, this);

}

Boolean FramedLiveSource::isFramedSource() const
{
	printf("FramedLiveSource running\n");
	return True;
}
Boolean FramedLiveSource::isH264VideoStreamFramer() const
{
	printf("FramedLiveSource running\n");
	return True;
}
