#include <stdio.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include "rwlock.h"
#include "common_types.h"

CRWLock::CRWLock()
{
	pthread_rwlockattr_t rwlock_attr;
	char msgbuf[KL_COMMON_BUF_SIZE];
	int res;

	if((res = pthread_rwlockattr_init(&rwlock_attr)) \
		!= 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_rwlockattr_init failed, " \
			"err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}

	if((res = pthread_rwlockattr_setpshared(&rwlock_attr, \
		PTHREAD_PROCESS_SHARED)) != 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"set rwlock attr to PTHREAD_PROCESS_SHARED " \
			"failed, err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}

	if((res = pthread_rwlock_init(&m_rwlock, \
		&rwlock_attr)) != 0){
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_rwlock_init failed, err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}

	pthread_rwlockattr_destroy(&rwlock_attr);
}

CRWLock::~CRWLock()
{
	pthread_rwlock_destroy(&m_rwlock);
}

int CRWLock::rdlock()
{
	return pthread_rwlock_rdlock(&m_rwlock);
}

int CRWLock::wrlock()
{
	return pthread_rwlock_wrlock(&m_rwlock);
}

int CRWLock::tryrdlock()
{
	return pthread_rwlock_tryrdlock(&m_rwlock);
}

int CRWLock::trywrlock()
{
	return pthread_rwlock_trywrlock(&m_rwlock);
}

int CRWLock::unlock()
{
	return pthread_rwlock_unlock(&m_rwlock);
}