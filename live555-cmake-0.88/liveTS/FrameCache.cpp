#include "FrameCache.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include "OSMutex.h"
#if defined(__WIN32__) || defined(_WIN32) || defined(__WIN32_WCE)
#include <windows.h>
#else
#include <unistd.h>
#endif
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#define DEFAULT_FRAME_SIZE 100*1024
#define MAX_FRAME_SIZE 2*1024*1024
FrameContainer::FrameContainer():buf(NULL),
					bufLen(0),contentLen(0),mFree(true)
{
	buf=new unsigned char[DEFAULT_FRAME_SIZE];
	bufLen=DEFAULT_FRAME_SIZE;
}
FrameContainer::~FrameContainer()
{

}
void FrameContainer::cleanContainer()
{
	/*just reset contentLen and container's stat*/
	contentLen=0; 
	setFreeStat(true);
}

int FrameContainer::writeData(unsigned char *buffer,unsigned int bufferLen)
{
	/*we have Max frame size  limit*/
	if(bufferLen>MAX_FRAME_SIZE){
		return 0;
	}
	/*we don't have enough buffer,so we should create new buffer that enough big to contain the data*/
	if(bufferLen>bufLen){
		bufLen+=bufferLen-bufLen;
		unsigned char *tBuf=new unsigned char[bufLen];
		delete[] buf; //delete old array
		buf=tBuf;
	}
	memcpy(buf,buffer,bufferLen);
	contentLen=bufferLen;

}
int FrameContainer::readData(unsigned char *&buffer,unsigned int &bufferLen)
{
		buffer=buf;
		bufferLen=contentLen;
		return 1;
}

#define DEFAULT_QUEUE_LEN 1
CacheManager::CacheManager():readQueue(NULL),readQueueLen(0),
									writeQueue(NULL),writeQueueLen(0),firstRead(true),
									mutex(new OSMutex)
{
	readQueue=new FrameContainer*[DEFAULT_QUEUE_LEN];
	readQueueLen=DEFAULT_QUEUE_LEN;
	for(int i=0;i<readQueueLen;++i){
		readQueue[i]=new FrameContainer;
	}
	writeQueue=new FrameContainer*[DEFAULT_QUEUE_LEN];
	writeQueueLen=DEFAULT_QUEUE_LEN;
	for(int i=0;i<writeQueueLen;++i){
		writeQueue[i]=new FrameContainer;
	}
}
CacheManager::~CacheManager()
{
	for(int i=0;i<readQueueLen;++i){
		delete readQueue[i];
		readQueue[i]=NULL;
	}
	delete readQueue;
	for(int i=0;i<writeQueueLen;++i){
		delete writeQueue[i];
		writeQueue[i]=NULL;
	}
	delete writeQueue;
}

void CacheManager::swapQueue()
{

	/*swap readQueue and writeQueue*/
  	mutex->Lock();
  	FrameContainer **tReadQueue=readQueue;
  	readQueue=writeQueue;
  	writeQueue=tReadQueue;

	int tLen=readQueueLen;
  	readQueueLen=writeQueueLen;
	writeQueueLen=tLen;
    mutex->Unlock();
}
void CacheManager::cleanCache()
{
	/*clean readQueue and writeQueue*/
	for(int i=0;i<readQueueLen;++i){
		readQueue[i]->cleanContainer();
		readQueue[i]->setFreeStat(true);
	}
	for(int i=0;i<writeQueueLen;++i){
		writeQueue[i]->cleanContainer();
		writeQueue[i]->setFreeStat(true);
	}
}
int CacheManager::writeData(unsigned char *buffer,unsigned int bufferLen)
{
	if(bufferLen<=0){
		return 0;
	}
	
	//fprintf(stderr,"%s %d bufferLen %d\n",__FUNCTION__,__LINE__,bufferLen);
	mutex->Lock();
	//fprintf(stderr,"writeQueueLen %d \n",writeQueueLen);
	for(int i=0;i<writeQueueLen;++i){
		FrameContainer *fc=writeQueue[i];
		if(fc->getFreeStat()){
			fc->writeData(buffer,bufferLen);
			fc->setFreeStat(false);
			mutex->Unlock();	
			return 1;
		}
	}
	#if 1
	for(int i=0;i<writeQueueLen;++i){
		writeQueue[i]->cleanContainer();
		writeQueue[i]->setFreeStat(true);
	}	
	#endif
	mutex->Unlock();
	return 0;
}
int CacheManager::readData(unsigned char *&buffer,unsigned int &bufferLen)
{
  if(firstRead){ //we
     cleanCache();
    firstRead=false;
  }
  //fprintf(stderr,"%s %d \n",__FUNCTION__,__LINE__);

  /*get one frame*/
  for(int i=0;i<readQueueLen;++i){
		if(!readQueue[i]->getFreeStat()){
			readQueue[i]->readData(buffer,bufferLen);
	//		fprintf(stderr,"%s %d %d\n",__FUNCTION__,__LINE__,bufferLen);
			readQueue[i]->setFreeStat(true);
			return 1;

		}
  }
  /*not get frame,swap queue*/
  swapQueue();
 // fprintf(stderr,"%s %d \n",__FUNCTION__,__LINE__);

  /*again*/
  for(int i=0;i<readQueueLen;++i){
		if(!readQueue[i]->getFreeStat()){
			readQueue[i]->readData(buffer,bufferLen);
			readQueue[i]->setFreeStat(true);
		}
		return 1;
  }
  
  return 0;
}

