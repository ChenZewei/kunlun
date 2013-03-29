#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "global.h"
#include "epoll_engine.h"

CEpollEngine::CEpollEngine()
{
	if(open(KL_COMMON_EPOLL_FD_SIZE) !=0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"create epoll fd failed, err: %s", \
			__LINE__, strerror(errno));
		return;
	}
	m_timeout = -1;
	memset(m_events, 0, sizeof(m_events));
}

CEpollEngine::CEpollEngine(int size, int timeout)
{
	if(open(size) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"create epoll fd failed, err: %s", \
			__LINE__, strerror(errno));
		return;
	}
	m_timeout = timeout;
	memset(m_events, 0, sizeof(m_events));
}

CEpollEngine::~CEpollEngine()
{
	//首先需要将list中未被解除绑定的观察者全部解除
	//再将对象删除
	std::list<CSockObserver*>::iterator iter;
	CSockObserver *psock_ob;
	for(iter = m_ObList.begin(); iter != m_ObList.end(); iter++){
		psock_ob = *(iter);
		if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, psock_ob->get_fd(), \
			NULL) != 0)
		{
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"detach fd: %d failed, err: %s", \
				__LINE__, psock_ob->get_fd(), strerror(errno));
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
	if(m_epfd == -1 || psock_observer == NULL)
		return ;

	ev.data.ptr = (void*)psock_observer;
	ev.events = nstatus;

#ifdef _DEBUG
	printf("sock_observer (fd: %d) attach with epoll (fd: %d), status: %d", \
		psock_observer->get_fd(), m_epfd, nstatus);
#endif //_DEBUG

	if(epoll_ctl(m_epfd, EPOLL_CTL_ADD, psock_observer->get_fd(), \
		&ev) != 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"attach fd: %d failed, err: %s", \
			__LINE__, psock_observer->get_fd(), strerror(errno));
		return -1;
	}
	
	m_ObList.push_back(psock_observer);
	return 0;
}

int CEpollEngine::detach(CSockObserver *psock_observer)
{
	int res;
	if(m_epfd == -1 || psock_observer == NULL)
		return ;

#ifdef _DEBUG
	printf("sock_observer (fd: %d) detach with epoll (fd: %d)", \
		psock_observer->get_fd(), m_epfd);
#endif //_DEBUG

	if(epoll_ctl(m_epfd, EPOLL_CTL_DEL, psock_observer->get_fd(), \
		NULL) != 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"detach fd: %d failed, err: %s", \
			__LINE__, psock_observer->get_fd(), strerror(errno));
	}
	m_ObList.remove(psock_observer);
	return res;
}

void CEpollEngine::Stop()
{
	m_stop_flag = true;
}

void CEpollEngine::run()
{
	int nfds;
	int i;
	CSockObserver *psock_ob;

	m_stop_flag = false;
	while(m_stop_flag != true){
		nfds = epoll_wait(m_epfd, m_events, \
			KL_COMMON_EPOLL_FD_SIZE, m_timeout);
		if(nfds == -1){
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"epoll_wait was interrupted, err: %s", \
				__LINE__, strerror(errno));
			break;
		}

		for(i = 0; i < nfds; i++){
			psock_ob = (CSockObserver*)(m_events[i].data.ptr);
			psock_ob->work(this, m_events[i].events);
		}
	}
	KL_SYS_NOTICELOG("file: "__FILE__", line: %d, " \
		"epoll engine stop to work", __LINE__);
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
		KL_SYS_WARNNINGLOG("file: "__LINE__", line: %d, " \
			"set fd: %d to status: %d failed, err: %s", \
			__LINE__, nstatus, strerror(errno));
		return -1;
	}
	return 0;
}
