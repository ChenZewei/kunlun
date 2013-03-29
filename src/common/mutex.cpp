#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mutex.h"
#include "common_types.h"

CMutex::CMutex()
{
	char msgbuf[KL_COMMON_BUF_SIZE];
	pthread_mutexattr_t mat;
	int res;

	if((res = pthread_mutexattr_init(&mat)) != 0){
		//kl_errout("pthread_mutexattr_init failed");
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_mutexattr_init failed, " \
			"err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}

	if((res = pthread_mutexattr_settype(&mat, \
		PTHREAD_MUTEX_ERRORCHECK_NP)) != 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_mutexattr_settype failed, " \
			"err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}

	if((res = pthread_mutex_init(&m_mutex, &mat)) != 0){
		//kl_errout("pthread_mutex_init failed");
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, "\
			"call pthread_mutex_init failed, err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}

	pthread_mutexattr_destroy(&mat);
}

CMutex::~CMutex()
{
	char msgbuf[KL_COMMON_BUF_SIZE];
	int res;
	res = pthread_mutex_destroy(&m_mutex);
	if(res != 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d ," \
			"call pthread_mutex_destroy failed, " \
			"err: %s", \
			__LINE__, strerror(res));
		printf("%s\n", msgbuf);
	}
}

int CMutex::lock()
{
	return pthread_mutex_lock(&m_mutex);
}

int CMutex::trylock()
{
	return pthread_mutex_trylock(&m_mutex);
}

int CMutex::unlock()
{
	return pthread_mutex_unlock(&m_mutex);
}