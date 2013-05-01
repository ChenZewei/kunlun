#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "epoll_engine.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CEpollEngine::CEpollEngine()
{
	if(open(KL_COMMON_EPOLL_FD_SIZE) == -1)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create epoll fd failed, err: %s", \
			__LINE__, strerror(errno));
		throw errno;
	}
	m_timeout = -1;
	memset(m_events, 0, sizeof(m_events));
}

CEpollEngine::CEpollEngine(int size, int timeout)
{
	if(open(size) != -1)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create epoll fd failed, err: %s", \
			__LINE__, strerror(errno));
		throw errno;
	}
	m_timeout = timeout;
	memset(m_events, 0, sizeof(m_events));
}

/*
 * @description: before deleting epoll, need to detach epoll with
                 the object of observer, and delete all observer lefted
				 in the ob_list
 */
CEpollEngine::~CEpollEngine()
{
	std::list<CSockObserver*>::iterator iter;
	CSockObserver *psock_ob;
	for(iter = m_ob_list.begin(); iter != m_ob_list.end(); iter++){
		psock_ob = *(iter);
		if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, psock_ob->get_fd(), \
			NULL) != 0)
		{
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"detach fd: %d failed, err: %s", \
				__LINE__, psock_ob->get_fd(), strerror(errno));
			throw errno;
		}
		delete psock_ob;
		psock_ob = NULL;
	}
	close(m_epfd);
}

int CEpollEngine::open(int size)
{
	m_epfd = epoll_create(size);
	return m_epfd;
}

int CEpollEngine::attach(CSockObserver *psock_observer, uint32_t nstatus)
{
	struct epoll_event ev;
	ev.data.ptr = (void*)psock_observer;
	ev.events = nstatus;

#ifdef _DEBUG
	assert(m_epfd > 0 && psock_observer);
	KL_SYS_DEBUGLOG("sock_observer(fd: %d) attach with epoll(fd: %u), status: %d", \
		psock_observer->get_fd(), m_epfd, nstatus);
#endif //_DEBUG

	if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, psock_observer->get_fd(), \
		&ev) != 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"sock_observer(fd: %d) attach with epoll(fd: %d) failed, err: %s", \
			__LINE__, psock_observer->get_fd(), strerror(errno));
		return -1;
	}
	
	m_ob_list.push_back(psock_observer);
	return 0;
}

int CEpollEngine::detach(CSockObserver *psock_observer)
{
	int res;

#ifdef _DEBUG
	assert(m_epfd > 0 && psock_observer);
	KL_SYS_DEBUGLOG("sock_observer(fd: %d) detach with epoll(fd: %d)", \
		psock_observer->get_fd(), m_epfd);
#endif //_DEBUG

	if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, psock_observer->get_fd(), \
		NULL) != 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"sock_observer(fd: %d) detach with epoll(fd: %d) failed, err: %s", \
			__LINE__, psock_observer->get_fd(), strerror(errno));
	}
	m_ob_list.remove(psock_observer);
	return res;
}

int CEpollEngine::stop()
{
	m_stop_flag = true;
	return 0;
}

int CEpollEngine::notify()
{
	int nfds;
	int i;
	CSockObserver *psock_ob;

	m_stop_flag = false;
	while(m_stop_flag != true)
	{
		nfds = epoll_wait(m_epfd, m_events, KL_COMMON_EPOLL_FD_SIZE, m_timeout);
		if(nfds == -1)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"epoll_wait was interrupted, epoll engine stop to work, err: %s", \
				__LINE__, strerror(errno));
			return -1;
		}

		for(i = 0; i < nfds; i++)
		{
			psock_ob = (CSockObserver*)(m_events[i].data.ptr);
			psock_ob->work(this, m_events[i].events);
		}
	}
	KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
		"epoll engine stop to work", __LINE__);
	return 0;
}

int CEpollEngine::set_ob_status(CSockObserver *psock_observer, uint32_t nstatus)
{
	struct epoll_event ev;
	if(m_epfd == -1 || psock_observer == NULL)
		return -1;

	ev.data.ptr = (void*)psock_observer;
	ev.events = nstatus;
	if(epoll_ctl(m_epfd, EPOLL_CTL_MOD, psock_observer->get_fd(), \
		&ev) != 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"set sock_observer(fd: %d) to status: %u failed, err: %s", \
			__LINE__, nstatus, strerror(errno));
		return -1;
	}
	return 0;
}
