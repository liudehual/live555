#ifndef _FRAMESMANAGER_H_
#define _FRAMESMANAGER_H_

class FramesManager{
public:
	FramesManager(int ringBufferSize);
	~FramesManager();
public:
	//��ʼ��ȡ����
	int Start();
	//������ȡ����
	int Stop();
	int WriteFrame(unsigned char *data, int len);
	int ReadFrame(unsigned char **data, int maxlen);
	int ReadFrameCopied(unsigned char *data, int maxlen);
	//��ȡʣ��ռ�
	int GetFree(void);
	int m_len;//len of ring buffer
	long long lastUsec;
private:
	unsigned char *m_Ringbuffer;
	int m_wPtr; //write ptr
	int m_rPtr; // read ptr
	bool m_bStarted;
};

#endif
