#include <pthread.h>
#include <string.h>
#include <time.h>
#include <strings.h>
#include "cond.h"
#include "common_types.h"

CCond::CCond()
{
	int res;
	char msgbuf[KL_COMMON_BUF_SIZE];
	pthread_condattr_t cond_attr;
	pthread_condattr_init(&cond_attr);
	if((res = pthread_cond_init(&m_cond, \
		&cond_attr)) != 0)
	{
		bzero(msgbuf, KL_COMMON_BUF_SIZE);
		snprintf(msgbuf, KL_COMMON_BUF_SIZE, \
			"file: "__FILE__", line: %d, " \
			"call pthread_cond_init failed, err: %s", \
			__LINE__, strerror(res));
		kl_errout(msgbuf);
	}
	pthread_condattr_destroy(&cond_attr);
}

CCond::~CCond()
{
	pthread_cond_destroy(&m_cond);
}

int CCond::wait(CMutex *mutex_ptr)
{
	return pthread_cond_wait(&m_cond, \
		&(mutex_ptr->m_mutex));
}

int CCond::timed_wait(CMutex *mutex_ptr, int timeout)
{
	struct timeval now;
	struct timespec time_out;

	gettimeofday(&now);
	time_out.tv_sec = now.tv_sec + timeout;
	time_out.tv_nsec = now.tv_usec * 1000;

	return pthread_cond_wait(&m_cond, \
		&(mutex_ptr->m_mutex), &time_out);
}

int CCond::wakeup()
{
	return pthread_cond_signal(&m_cond);
}

int CCond::wakeup_all()
{
	return pthread_cond_broadcast(&m_cond);
}