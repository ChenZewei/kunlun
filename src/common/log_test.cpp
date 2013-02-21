#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "log.h"

int gogo = 0;

void* logthread(void *args)
{
	CLog *pLog = (CLog *)args;
	int i;
	int num = 10;
	for(i = 0; i < 128; i++){
		pLog->WriteLog(LOG_LEVEL_ERROR, \
			"write error level %d", num);
		pLog->WriteLog(LOG_LEVEL_WARNNING, \
			"write warnning level %d", num);
		pLog->WriteLog(LOG_LEVEL_NOTICE, \
			"write notice level %d", num);
		pLog->WriteLog(LOG_LEVEL_INFO, \
			"write info level %d", num);

		num += 10;
		pLog->WriteLog2(LOG_LEVEL_ERROR, \
			"write error level %d", num);
		pLog->WriteLog2(LOG_LEVEL_WARNNING, \
			"write warnning level %d", num);
		pLog->WriteLog2(LOG_LEVEL_NOTICE, \
			"write notice level %d", num);
		pLog->WriteLog2(LOG_LEVEL_INFO, \
			"write info level %d", num);
	}
	gogo++;
	return NULL;
}

int main(int argc, char *argv[])
{
	CLog *pLog = new CLog("a.log", LOG_LEVEL_NOTICE);
	pthread_t pid;
	int i;
	if(pLog == NULL){
		printf("create log file failed\n");
		return -1;
	}

	for(i = 0; i < 10; i++){
		pthread_create(&pid, NULL, logthread, pLog);
	}
	
	while(gogo < 10){
		sleep(1);
	}

	printf("write log done\n");
	delete pLog;
	return 0;
}