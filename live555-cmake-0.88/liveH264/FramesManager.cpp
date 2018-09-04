#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FramesManager.h"

#ifdef WIN32
#include "windows.h"
#else
#include <unistd.h>
#endif

FramesManager::FramesManager(int ringBufferSize) {
	m_bStarted = false;
	m_len = ringBufferSize;
	m_wPtr = 0;
	m_rPtr = 0;
	m_Ringbuffer = new unsigned char [m_len];
}

FramesManager::~FramesManager() {
	m_bStarted = false;
#ifdef WIN32
	Sleep(100);
#else
	usleep(100*1000);
#endif
	delete []m_Ringbuffer;
}
int FramesManager::WriteFrame(unsigned char *data, int len)
{
	int m_wSize;
	if (!m_bStarted)
		return -1;
	if (m_Ringbuffer == NULL)
		return -1;
	m_wSize = (m_wPtr + m_len - m_rPtr) % m_len;
	if (m_wSize + len > m_len){
		return -1;
	}

	if (m_wPtr >= m_len)
		m_wPtr = 0;
	if (m_wPtr + len > m_len){
		int temp = m_len - m_wPtr;
		memcpy(&m_Ringbuffer[m_wPtr], data, temp);
		data += temp;
		len -= temp;
		m_wPtr = 0;
	}
	memcpy(&m_Ringbuffer[m_wPtr], data, len);
	m_wPtr += len;
	return 0;
}

int FramesManager::Start() 
{
	m_wPtr = 0;
	m_rPtr = 0;
	m_bStarted = true;
	return 0;
}

int FramesManager::Stop() 
{
	m_bStarted = false;
	return 0;
}

int FramesManager::ReadFrame(unsigned char **data, int maxlen)
{
	int tsize = (m_wPtr + m_len - m_rPtr) % m_len;
	if (!m_bStarted)
		return -1;
	if (tsize <= 0)
		return -1;
	if (m_rPtr >= m_len)
		m_rPtr = 0;
	*data = &m_Ringbuffer[m_rPtr];
	if (tsize < maxlen)
		maxlen = tsize;
	if (m_rPtr + maxlen > m_len){
		maxlen = m_len - m_rPtr;
	}
	m_rPtr += maxlen;
	return maxlen;
}

int FramesManager::ReadFrameCopied(unsigned char *data, int maxlen)
{
	unsigned char *p;
	int tsize = (m_wPtr + m_len - m_rPtr) % m_len;
	if (!m_bStarted)
		return -1;
	if (tsize <= 0)
		return -1;
	if (m_rPtr >= m_len)
		m_rPtr = 0;
	p = &m_Ringbuffer[m_rPtr];
	if (tsize < maxlen)
		maxlen = tsize;
	if (m_rPtr + maxlen > m_len){
		int temp = m_len - m_rPtr;
		memcpy(data, p, temp);
		m_rPtr = maxlen - temp;
		memcpy(&data[temp], m_Ringbuffer, m_rPtr);
	}else{
		memcpy(data, p, maxlen);
		m_rPtr += maxlen;
	}
	return maxlen;
}

int FramesManagerTest(void)
{
	int j;
	int i;
	unsigned char data[1024];
	unsigned char *w;
	unsigned char tp[1024];
	int len;
	FramesManager *f = new FramesManager(53);
	f->Start();
	w = (unsigned char *)data;
	for (i=0;i<1024;i++){
		data[i] = (unsigned char)i;
	}
	for (j=0;j<15;j++){
		printf("one time\n");
		f->WriteFrame(w, 32);
		w += 32;
		len = f->ReadFrameCopied(tp, 32);
		printf("readed: %d\n", len);
		for (i=0;i<len;i++){
			printf("%02x,", tp[i]);
		}
		printf("\n");
		len = f->ReadFrameCopied(tp, 32);
		for (i=0;i<len;i++){
			printf("%02x,", tp[i]);
		}
		printf("\n");
	}
	return 0;
}

int FramesManager::GetFree(void) {
	return m_len - ((m_wPtr + m_len - m_rPtr) % m_len);
}
