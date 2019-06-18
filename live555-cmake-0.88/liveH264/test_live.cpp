#include "Live.h"
#include "LiveClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#if	defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
	#include <windows.h>
#else
	#include <sys/time.h>
	#include <string.h>
	#include <unistd.h>
#endif
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
#pragma comment(lib,"ws2_32.lib")
#endif
static pthread_t LiveThread;
static void *Live_thread(void *param)
{
	/*
		定义每个LiveServerMediaSubSession buffer 大小
		有几个LiveServerMediaSubSession 就定义多大的bufferSize
	*/
	int bufSize[] = {3*1024*1024, 1*1024*1024, 512*1024}; 
	/*
		定义每个LiveServerMediaSubSession 帧率
	*/
	int frameList[] = {25,25,25};
	liveRtspStart("live%d.264", 8554, bufSize, frameList, 3);
	return 0;
}

int main(int argc, char *argv[])
{
	int readPtr = 500*1024;
	int leftCnt;
	int haveStartcode = 0;
	
	pthread_create(&LiveThread, NULL,Live_thread, NULL);

	int a = 10;
	int i;
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
	Sleep(50);
#else
	usleep(50*1000);
#endif
	while(1){	
		int len;
		static unsigned char buffer[5*1024*1024];
		static FILE *fp = NULL;
		if (fp == NULL){
			fp = fopen("/home/gct/share/test.h264", "rb");
		}
		
		if (fp == NULL){
			fprintf(stderr,"fopen Error \n");
		    return -1;
        }
        
		leftCnt = 5*1024*1024 - readPtr;
		memmove(buffer, &buffer[readPtr], leftCnt);

		len = fread(&buffer[leftCnt], 1, readPtr, fp);
		if (len <= 0 || feof(fp)){
			fseek(fp, 0, SEEK_SET);
			printf("continue----------------------------------------------\n");
			continue;
		}

		len += leftCnt;
		readPtr = 0;
		
		while(readPtr < len){
			int nextStartCodePtr;
			nextStartCodePtr = readPtr;
			nextStartCodePtr += 4;

			while(nextStartCodePtr < len){
				if (buffer[nextStartCodePtr+0] == 0
						&& buffer[nextStartCodePtr+1] == 0
						&& buffer[nextStartCodePtr+2] == 0
						&& buffer[nextStartCodePtr+3] == 1){
					liveRtspWriteEx(0, (char *)&buffer[readPtr], nextStartCodePtr - readPtr);
					liveRtspWriteEx(1, (char *)&buffer[readPtr], nextStartCodePtr - readPtr);
					liveRtspWriteEx(2, (char *)&buffer[readPtr], nextStartCodePtr - readPtr);
					
					readPtr = nextStartCodePtr;
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
					Sleep(50);
#else
					usleep(30*1000);
#endif
					break;
				}
				nextStartCodePtr++;
			}
			if (nextStartCodePtr >= len)
				break;
		}
	}
	return 0;
}
