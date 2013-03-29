#include <stdio.h>
#include "mutex.h"
#include "event.h"
#include "msg_queue.h"

CMsgQueue::CMsgQueue()
{
	m_ppush_mutex = new CMutex();
	m_ppop_mutex = new CMutex();
	m_pmsg_event = new CEvent(true);
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
	m_ppush_mutex->lock();
	m_msg_queue.push(pkg_msg_ptr);
	m_ppush_mutex->unlock();
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