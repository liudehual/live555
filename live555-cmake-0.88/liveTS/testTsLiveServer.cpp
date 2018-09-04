#if defined(__WIN32__) || defined(_WIN32) || defined(__WIN32_WCE)
#include <windows.h>
#include <process.h>
#else
#include<pthread.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "LiveTs.h"
void *WirteDataThread(void *arg)
{
	startRtspServer(12345);
}
int main(int argc,char *argv[])
{
	pthread_t thread1;
	pthread_create(&thread1,NULL,WirteDataThread,NULL);

	sleep(1);
	setChannel(16);
	sleep(1);

	#if 1
	FILE *fp=fopen("test.ts","rb");
	if(fp==NULL){
		fprintf(stderr,"Open File Error \n");
		exit(1);
	}
	unsigned char buffer[188*7];
	while(1){
		int len=fread(buffer,1,188*7,fp);
		if (len <= 0 || feof(fp)){
			fseek(fp, 0, SEEK_SET);
			printf("continue----------------------------------------------\n");
			continue;
		}
		writeData(buffer,len,0);
		writeData(buffer,len,1);
		writeData(buffer,len,2);
		writeData(buffer,len,3);
		writeData(buffer,len,4);
		writeData(buffer,len,5);
		writeData(buffer,len,6);

		writeData(buffer,len,7);

		usleep(20*1000);
	}
	#endif

	for(;;){
		sleep(1);
	}
	fprintf(stderr,"hello testTsLiveServer \n");
	return 0;
}
