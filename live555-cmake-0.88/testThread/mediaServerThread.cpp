#include "liveThread.hh"
#include <pthread.h>
#include <time.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
#pragma comment(lib,"ws2_32.lib")
#endif

int main()
{
	/*
		ÎÒÃÇ×¢ÊÍµô 554 7070 ¶Ë¿Ú£¬·ÀÖ¹ÓëDSS ³åÍ»
	*/
#if 0
	pthread_t t1;
	LiveThread thread1(554);
	pthread_create(&t1,NULL,LiveThread::Run,&thread1);

	pthread_t t2;
	LiveThread thread2(7070);
    pthread_create(&t2,NULL,LiveThread::Run,&thread2);
#endif

	pthread_t t3;
	LiveThread thread3(12345);
    pthread_create(&t3,NULL,LiveThread::Run,&thread3);

	pthread_t t4;
	LiveThread thread4(12346);
    pthread_create(&t4,NULL,LiveThread::Run,&thread4);

	pthread_t t5;
	LiveThread thread5(33330);
	pthread_create(&t5,NULL,LiveThread::Run,&thread5);
	
	pthread_t t6;
	LiveThread thread6(33331);
    pthread_create(&t6,NULL,LiveThread::Run,&thread6);

	pthread_t t7;
	LiveThread thread7(33332);
    pthread_create(&t7,NULL,LiveThread::Run,&thread7);

	pthread_t t8;
	LiveThread thread8(33333);
    pthread_create(&t8,NULL,LiveThread::Run,&thread8);

	while(1){
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
		Sleep(3000);
#else
		sleep(3);
#endif
	}
	return 1;
}
