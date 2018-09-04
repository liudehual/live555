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
#ifndef _FRAMED_LIVE_SOURCE_HH
#define _FRAMED_LIVE_SOURCE_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "FramedSource.hh"
#endif
#include "FramesManager.h"

class FramedLiveSource: public FramedSource {
public:
	FramedLiveSource(UsageEnvironment& env, FramesManager *dataManager);
	~FramedLiveSource();
public:
	virtual unsigned maxFrameSize() const;
	virtual void doGetNextFrame();
	virtual Boolean isFramedSource() const;
	virtual Boolean isH264VideoStreamFramer() const;

private:
	static void doGetNextFrame0(void* clientData);
public:
	  FramesManager *dataManager;
};
#endif
