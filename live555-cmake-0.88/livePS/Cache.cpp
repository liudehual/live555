#include "Cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__WIN32__) || defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif
#define UNUSED_CACHE_SIZE 1048576 // 1M
Cache::Cache() {

	m_len = 5*1024*1024;

	m_Ringbuffer = new char[m_len];
	m_wPtr = m_Ringbuffer;
	m_rPtr = m_Ringbuffer;
	
	pthread_mutex_init(&cache_mutex,NULL);
	pthread_cond_init(&cache_cond,NULL);
}

Cache::~Cache() {
#if defined(__WIN32__) || defined(_WIN32)
	Sleep(100);
#else
	usleep(100*1000);
#endif
	delete []m_Ringbuffer;
}
int Cache::write_data(unsigned char *data, int len)
{
	if(!data || len <=0 || len >=UNUSED_CACHE_SIZE/*>1M error*/){
		printf("[%s][%d] %p len %d \n",__FUNCTION__,__LINE__,data,len);
		return -1;
	}
	pthread_mutex_lock(&cache_mutex);
	int unused_data_len=m_Ringbuffer+m_len-m_wPtr;
	if(unused_data_len>=len){
		memcpy(m_wPtr,data,len);
		m_wPtr+=len;
		pthread_mutex_unlock(&cache_mutex);
		return 0;
	}
	if(m_wPtr<m_rPtr){
		m_wPtr=m_rPtr;
	}

	// ÄÚ´æ²»×ã£¬µÈ´ý
	if(m_Ringbuffer+m_len-m_wPtr<len){
		pthread_cond_wait(&cache_cond,&cache_mutex);
	}
	
	if(m_Ringbuffer+m_len-m_wPtr<len){
		pthread_mutex_unlock(&cache_mutex);
		return -2;
	}
	memcpy(m_wPtr,data,len);
	m_wPtr+=len;
	pthread_mutex_unlock(&cache_mutex);
	return 0;
}


int Cache::read_data(unsigned char *data,int in_len ,int &out_len)
{
	pthread_mutex_lock(&cache_mutex);

	if(m_rPtr>=m_wPtr){
		m_rPtr=m_wPtr=m_Ringbuffer;
		pthread_cond_signal(&cache_cond);
		return -1;
	}
	int using_data_len=m_wPtr-m_rPtr;

	if(using_data_len>=in_len){
		memcpy(data,m_rPtr,in_len);
		m_rPtr+=in_len;
		out_len=in_len;
	}
	if(using_data_len<in_len){
		memcpy(data,m_rPtr,using_data_len);
		m_rPtr+=using_data_len;
		out_len=using_data_len;
	}
	if(m_Ringbuffer+m_len-m_wPtr<UNUSED_CACHE_SIZE){
		int used_data_len=m_rPtr-m_Ringbuffer;
		int using_data_len=m_wPtr-m_rPtr;
		memmove(m_Ringbuffer,m_rPtr,using_data_len);
		m_wPtr-=used_data_len;
		m_rPtr=m_Ringbuffer;
	}
	if(m_Ringbuffer+m_len-m_wPtr>UNUSED_CACHE_SIZE){
		pthread_cond_signal(&cache_cond);
	}	
	pthread_mutex_unlock(&cache_mutex);
	return 0;
}


int Cache::get_free(void) {
	return 0;
}


