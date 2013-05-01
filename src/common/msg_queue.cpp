#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <new>
#include "log.h"
#include "mutex.h"
#include "event.h"
#include "msg_queue.h"

CMsgQueue::CMsgQueue()
{
	try
	{
		m_ppush_mutex = new CMutex();
	}
	catch(std::bad_alloc)
	{
		m_ppush_mutex = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CMutex constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		throw errcode;
	}
	if(m_ppush_mutex == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create push mutex", \
			__LINE__);
		throw ENOMEM;
	}

	try
	{
		m_ppop_mutex = new CMutex();
	}
	catch(std::bad_alloc)
	{
		m_ppop_mutex = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CMutex constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		throw errcode;
	}
	if(m_ppop_mutex == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create pop mutex", \
			__LINE__);
		throw ENOMEM;
	}

	try
	{
		m_pmsg_event = new CEvent(true);
	}
	catch(std::bad_alloc)
	{
		m_pmsg_event = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CEvent constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		throw errcode;
	}
	if(m_pmsg_event == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create msg event", \
			__LINE__);
		throw ENOMEM;
	}
}

CMsgQueue::~CMsgQueue()
{
	pkg_message *pkg_msg_ptr;

	if(m_ppush_mutex != NULL)
	{
		delete m_ppush_mutex;
		m_ppush_mutex = NULL;
	}
	if(m_ppop_mutex != NULL)
	{
		delete m_ppop_mutex;
		m_ppop_mutex = NULL;
	}
	if(m_pmsg_event != NULL)
	{
		delete m_pmsg_event;
		m_pmsg_event = NULL;
	}

	while(!m_msg_queue.empty())
	{
		pkg_msg_ptr = pop();
		if(pkg_msg_ptr != NULL)
			delete pkg_msg_ptr;
	}
}

void CMsgQueue::push_msg(pkg_message *pkg_msg_ptr)
{
	push(pkg_msg_ptr);
	m_pmsg_event->signal_set();
}

pkg_message* CMsgQueue::get_msg()
{
	m_pmsg_event->signal_wait();
	return pop();
}

void CMsgQueue::push(pkg_message *pkg_msg_ptr)
{
	int res;
	if((res = m_ppush_mutex->lock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"lock the push mutex failed", \
			__LINE__, strerror(res));
		return;
	}
	m_msg_queue.push(pkg_msg_ptr);
	if((res = m_ppush_mutex->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock the push mutex failed", \
			__LINE__, strerror(res));
	}
}

pkg_message* CMsgQueue::pop()
{
	pkg_message *pkg_msg_ptr;

	pkg_msg_ptr = NULL;
	m_ppop_mutex->lock();
	pkg_msg_ptr = m_msg_queue.front();
	m_msg_queue.pop();
	m_ppop_mutex->unlock();

	return pkg_msg_ptr;
}