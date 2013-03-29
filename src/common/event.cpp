#include <string.h>
#include <errno.h>
#include "mutex.h"
#include "cond.h"
#include "event.h"
#include "log.h"
#include "global.h"

CEvent::CEvent() : m_nsignal(0), m_signal_cond(), \
	m_bsemaphore(false), m_signal_mutex()
{
}

CEvent::CEvent(bool bsemaphore) : m_nsignal(0), \
	m_signal_cond(), m_bsemaphore(bsemaphore), m_signal_mutex()
{
}

CEvent::~CEvent()
{
}

int CEvent::signal_set()
{
	int res;
	if((res = m_signal_mutex.lock()) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"lock m_signal_mutex failed, err: %s", \
			__LINE__, strerror(res));
		return res;
	}

	m_nsignal ++;
	if((res = m_signal_mutex.unlock()) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"unlock m_signal_mutex failed, err: %s", \
			__LINE__, strerror(res));
		return res;
	}

	if((res = m_signal_cond.wakeup()) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"wake up event failed, err: %s", \
			__LINE__, strerror(res));
		return res;
	}

	return 0;
}

int CEvent::signal_wait()
{
	int res;

	if((res = m_signal_mutex.lock()) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"lock m_signal_mutex failed, err: %s", \
			__LINE__, strerror(res));

		return res;
	}

	while(m_nsignal == 0)
	{
		m_signal_cond.wait(&m_signal_mutex); //always success
	}
	if(m_bsemaphore)
		m_nsignal --;
	else
		m_nsignal = 0;

	if((res = m_signal_mutex.unlock()) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"unlock m_signal_mutex failed, err: %s", \
			__LINE__, strerror(res));
		return res;
	}

	return 0;
}