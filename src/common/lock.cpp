#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "lock.h"
#include "common_types.h"

CLock::CLock()
{
	char msgbuf[KL_COMMON_BUF_SIZE];
	pthread_mutexattr_t mat;
	int res;

	if(pthread_mutexattr_init(&mat) != 0){
		//kl_errout("pthread_mutexattr_init failed");
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_mutexattr_init failed, err", \
			__LINE__);
		kl_errout(msgbuf);
	}

	if((res = pthread_mutexattr_settype(&mat, \
		PTHREAD_MUTEX_ERRORCHECK_NP)) != 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_mutexattr_settype failed, err", \
			__LINE__);
		kl_errout(msgbuf);
	}

	if(pthread_mutex_init(&m_Mutex, &mat) != 0){
		//kl_errout("pthread_mutex_init failed");
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, "\
			"call pthread_mutex_init failed, err", \
			__LINE__);
		kl_errout(msgbuf);
	}

	pthread_mutexattr_destroy(&mat);
}

CLock::~CLock()
{
	char msgbuf[KL_COMMON_BUF_SIZE];
	int res;
	res = pthread_mutex_destroy(&m_Mutex);
	if(res != 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d ," \
			"call pthread_mutex_destroy failed, " \
			"err: %s\n", \
			__LINE__, strerror(res));
		printf("%s", msgbuf);
	}
}

int CLock::Lock()
{
	return pthread_mutex_lock(&m_Mutex);
}

int CLock::Unlock()
{
	return pthread_mutex_unlock(&m_Mutex);
}