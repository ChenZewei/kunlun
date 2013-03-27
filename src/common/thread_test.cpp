#include <stdio.h>
#include <unistd.h>
#include "thread.h"
#include "global.h"
#include "log.h"
#include "thread_func.h"

class CThreadExe : public CThreadFunc
{
public:
	virtual int run();
};

int CThreadExe::run()
{
	printf("------------------------\n");
	printf("thread1 start\n");
	printf("thread number: %d\n", pthread_self());
	printf("thread1 extends from CThreadFunc\n");
	printf("thread1 exit...\n");
	return 0;
}

class CThreadChild : public CThread
{
public:
	virtual int run();
};

int CThreadChild::run()
{
	printf("------------------------\n");
	printf("thread2 start\n");
	printf("thread number: %d\n", pthread_self());
	printf("thread2 extends from CThread\n");
	printf("thread2 exit...\n");
	return 0;
}

int main(int argc, char *argv[])
{
	//init put out log file
	g_psys_log = new CLog("sys_test.log", LOG_LEVEL_NOTICE);
	//must create on heap
	CThreadExe *pthread_exe = new CThreadExe();
	CThread *pthread1 = new CThread(pthread_exe, 0, true);
	pthread1->start();

	CThreadChild *pthread_child = new CThreadChild();
	pthread_child->start();

	while (true)
	{
		sleep(1);
	}

	return 0;
}