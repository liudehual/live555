#ifndef __LIVE_TS_H__
#define __LIVE_TS_H__

int startRtspServer(unsigned short port);
int setChannel(unsigned int chNum);
int writeData(unsigned char *buf,unsigned int bufLen,int channel);
#endif
