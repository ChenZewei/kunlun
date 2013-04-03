#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "acceptorOB.h"
#include "thread_msg_recv.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG
#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //USE_EPOLL
#include "epoll_engine.h"
#endif
#endif

CThreadMsgRecv::CThreadMsgRecv(CAcceptorOB *pacceptor_ob) : \
	m_pacceptor_ob(pacceptor_ob)
{
#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //default : USE_EPOLL
	m_pepoll_engine = new CEpollEngine();
	if(m_pepoll_engine == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory for server to create epoll engine object", \
			__LINE__);
		return;
	}
#endif //USE_SELECT
#endif //USE_POLL
}

CThreadMsgRecv::~CThreadMsgRecv()
{
	if(m_pacceptor_ob != NULL)
	{
		delete m_pacceptor_ob;
		m_pacceptor_ob = NULL;
	}
#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //USE_EPOLL
	if(m_pepoll_engine == NULL)
	{
		delete m_pepoll_engine;
		m_pepoll_engine = NULL;
	}
#endif
#endif
}

int CThreadMsgRecv::run()
{
	int res;
#ifdef _DEBUG
	assert(m_pacceptor_ob && m_pepoll_engine);
#endif //_DEBUG
#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //default : USE_EPOLL
	if((res = m_pepoll_engine->attach(m_pacceptor_ob, \
		EPOLLIN | EPOLLET)) != 0)
		return res;
	res = m_pepoll_engine->notify();
#endif //USE_POLL
#endif //USE_SELECT
	return res;
}

int CThreadMsgRecv::stop()
{
#ifdef _DEBUG
	assert(m_pepoll_engine);
#endif //_DEBUG
#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //default : USE_EPOLL
	return m_pepoll_engine->stop();
#endif //USE_POLL
#endif //USE_SELECT
}