#ifndef __LIVETHREAD_H_
#define __LIVETHREAD_H_
#include <BasicUsageEnvironment.hh>
#include "DynamicRTSPServer.hh"
class LiveThread
{
public:
  LiveThread(unsigned short port);
  ~LiveThread();
  UsageEnvironment *getEnv()const{return env;}
  void setPort(unsigned short tPort){mPort=tPort;}
  static void *Run(void *arg);	

private:
 TaskScheduler* scheduler;
 UsageEnvironment* env;
 RTSPServer* rtspServer;
 unsigned short mPort;
};

#endif
