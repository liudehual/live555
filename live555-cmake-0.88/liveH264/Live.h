#ifndef _JVLIVE_H_
#define _JVLIVE_H_

#ifdef __cplusplus
extern "C" {
#endif

//准备RTSP服务
//param nameFmt 类似live%d.264
//cnt 提供的视频的路数。
void liveRtspStart(char *nameFmt, unsigned short port, int *bufferSizeList, int *frameList, int cnt);

//audioType: "g726-40"/"PCMU"/"PCMA" ...
//PCMU，PCMA即对应的G711-U,-A. 目前，在8K,16BIT情况下，测试可用
void liveRtspStartEx(char *nameFmt, unsigned short port, int *bufferSizeList, int *frameList, int cnt, char *audioType, int timeStampFrequency);

//写入数据
int liveRtspWriteEx(int index, char *data, int len);

//写入数据
int liveRtspWrite(int index, char *data, int len);

//设置当前帧率
int liveRtspSetFramerate(int index, int framerate);

//写入音频数据
int liveRtspWrite_audio(int index, char *data, int len);

#ifdef __cplusplus
}
#endif

#endif
