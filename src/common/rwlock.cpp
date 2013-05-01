#include <stdio.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include "rwlock.h"

CRWLock::CRWLock()
{
	pthread_rwlockattr_t rwlock_attr;
	int res;

	if((res = pthread_rwlockattr_init(&rwlock_attr)) != 0)
	{
		printf("file: "__FILE__", line: %d, " \
			"call pthread_rwlockattr_init failed, err: %s\n", \
			__LINE__, strerror(res));
		throw res;
	}

	if((res = pthread_rwlockattr_setpshared(&rwlock_attr, \
		PTHREAD_PROCESS_SHARED)) != 0)
	{
		printf("file: "__FILE__", line: %d, " \
			"set rwlock attr to PTHREAD_PROCESS_SHARED failed, err: %s\n", \
			__LINE__, strerror(res));
		throw res;
	}

	if((res = pthread_rwlock_init(&m_rwlock, &rwlock_attr)) != 0)
	{
		printf("file: "__FILE__", line: %d, " \
			"call pthread_rwlock_init failed, err: %s\n", \
			__LINE__, strerror(res));
		throw res;
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