#ifndef _JVLIVE_H_
#define _JVLIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

//׼��RTSP����
//param nameFmt ����live%d.264
//cnt �ṩ����Ƶ��·����
void liveRtspStart(char *nameFmt, unsigned short port, int *bufferSizeList, int *frameList, int cnt);

//audioType: "g726-40"/"PCMU"/"PCMA" ...
//PCMU��PCMA����Ӧ��G711-U,-A. Ŀǰ����8K,16BIT����£����Կ���
void liveRtspStartEx(char *nameFmt, unsigned short port, int *bufferSizeList, int *frameList, int cnt, char *audioType, int timeStampFrequency);

//д������
int liveRtspWriteEx(int index, char *data, int len);

//д������
int liveRtspWrite(int index, char *data, int len);

//���õ�ǰ֡��
int liveRtspSetFramerate(int index, int framerate);

//д����Ƶ����
int liveRtspWrite_audio(int index, char *data, int len);

#ifdef __cplusplus
}
#endif

#endif
