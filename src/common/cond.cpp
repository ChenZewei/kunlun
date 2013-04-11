#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <strings.h>
#include "cond.h"
#include "log.h"
#include "common_types.h"

CCond::CCond()
{
	int res;
	pthread_condattr_t cond_attr;
	pthread_condattr_init(&cond_attr);
	if((res = pthread_cond_init(&m_cond, \
		&cond_attr)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call pthread_cond_init failed, err: %s", \
			__LINE__, strerror(res));
		return;
	}
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CCond constructor successfully");
#endif //_DEBUG
	pthread_condattr_destroy(&cond_attr);
}

CCond::~CCond()
{
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CCond destructor successfully");
#endif
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

	gettimeofday(&now, NULL);
	time_out.tv_sec = now.tv_sec + timeout;
	time_out.tv_nsec = now.tv_usec * 1000;

	return pthread_cond_timedwait(&m_cond, \
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