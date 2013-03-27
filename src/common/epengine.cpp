/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月11日 星期六 21时06分48秒
* File Name: epengine.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "epengine.h"

CEpEngine::CEpEngine()
{
	open(EPOLL_FD_SIZE);
	m_timeout = -1;
	memset(m_events, 0, sizeof(m_events));
}

CEpEngine::CEpEngine(int size, int timeout)
{
	open(size);
	m_timeout = timeout;
	memset(m_events, 0, sizeof(m_events));
}

CEpEngine::~CEpEngine()
{
	//首先需要将list中未被解除绑定的观察者全部解除
	//再将对象删除
	list<CObserver*>::iterator iter;
	CObserver *pOb;
	for(iter = m_ObList.begin(); iter != m_ObList.end(); iter++){
		pOb = *(iter);
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, pOb->GetFd(), NULL);
		delete pOb;
		pOb = NULL;
	}

	close(m_epfd);
}

int CEpEngine::open(int size)
{
	m_epfd = epoll_create(size);
	return m_epfd;
}

void CEpEngine::Attach(CObserver *pObserver, uint32_t nstatus)
{
	struct epoll_event ev;
	if(m_epfd == -1 || pObserver == NULL)
		return ;

	ev.data.ptr = (void*)pObserver;
	ev.events = nstatus;
	epoll_ctl(m_epfd, EPOLL_CTL_ADD, pObserver->GetFd(), &ev);
	
	m_ObList.push_back(pObserver);
}

void CEpEngine::Detach(CObserver *pObserver)
{
	if(m_epfd == -1 || pObserver == NULL)
		return ;

	epoll_ctl(m_epfd, EPOLL_CTL_DEL, pObserver->GetFd(), NULL);
	m_ObList.remove(pObserver);
}

void CEpEngine::Stop()
{
	m_stop_flag = true;
}

void CEpEngine::Run()
{
	int nfds;
	int i;
	CObserver *pOb;

	m_stop_flag = false;
	while(m_stop_flag != true){
		nfds = epoll_wait(m_epfd, m_events, EPOLL_FD_SIZE, m_timeout);
		if(nfds == -1){
			break;
		}

		for(i = 0; i < nfds; i++){
			pOb = (CObserver*)(m_events[i].data.ptr);
			pOb->Work(this, m_events[i].events);
		}
	}
}

void CEpEngine::SetOBStatus(CObserver *pObserver, uint32_t nstatus)
{
	struct epoll_event ev;
	if(m_epfd == -1 || pObserver == NULL)
		return ;

	ev.data.ptr = (void*)pObserver;
	ev.events = nstatus;
	epoll_ctl(m_epfd, EPOLL_CTL_MOD, pObserver->GetFd(), &ev);
}
