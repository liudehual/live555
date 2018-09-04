#ifndef __FRAME_CACHE_HH__
#define __FRAME_CACHE_HH__
#include <stdio.h>
#include <stdlib.h>

/* frame manager 2.0 */
class OSMutex;
/*
	�ô������������H264FrameManager�� �Ѳ⣬����
	ԭ��
		CacheManager ����readQueue
	new Code
	add by gct 1608091706
*/
class FrameContainer
{
	public:
		FrameContainer();
		~FrameContainer();
		void cleanContainer();
		void setFreeStat(bool tFree){mFree=tFree;}
		bool getFreeStat(){return mFree;}
		int writeData(unsigned char *buffer,unsigned int bufferLen);
		int readData(unsigned char *&buffer,unsigned int &bufferLen);

	private:
		unsigned char *buf;
		unsigned int bufLen; /*�������ܳ���*/
		unsigned int contentLen; /*���ݳ���*/
		bool mFree;
};
class CacheManager
{

	public:
		CacheManager();
		~CacheManager();
		int writeData(unsigned char *buffer,unsigned int bufferLen);
		int readData(unsigned char *&buffer,unsigned int &bufferLen);

		void swapQueue();
		void cleanCache();
	private:
		FrameContainer **readQueue;
		int readQueueLen;
		FrameContainer **writeQueue;
		int writeQueueLen;
		OSMutex *mutex;                 //ͬ����
		bool firstRead;
};
#endif
