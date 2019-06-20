#ifndef __CACHE_H__
#define __CACHE_H__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

class Cache
{
public:
	Cache();
	~Cache();
public:
	int write_data(unsigned char *data, int len);
	int read_data(unsigned char *data,int in_len ,int &out_len);
	//ªÒ»° £”‡ø’º‰
	int get_free(void);
private:
	char *m_Ringbuffer;
	char *m_wPtr; //write ptr
	char *m_rPtr; // read ptr
	int m_len;
	pthread_mutex_t cache_mutex;
	pthread_cond_t cache_cond;
};




#endif
