#ifndef THREAD_MSG_RECV_H_
#define THREAD_MSG_RECV_H_
#include "thread_func.h"

#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //default : USE_EPOLL
class CEpollEngine;
#endif //USE_POLL
#endif //USE_SELECT

class CAcceptorOB;
/*
 * @description: CThreadMsgRecv provide executive func to recv connected
				 request and data request, a acceptor observer is needed,
				 acceptor is dispatched by base server and deleted in here
 */
class CThreadMsgRecv : public CThreadFunc
{
public:
	CThreadMsgRecv(CAcceptorOB *pacceptor_ob);
	~CThreadMsgRecv();

	int run();
	int stop();
private:
	CAcceptorOB *m_pacceptor_ob;
#ifdef USE_SELECT

#else
#ifdef USE_POLL

#else //USE_EPOLL
	CEpollEngine *m_pepoll_engine;
#endif
#endif
};
#endif //THREAD_MSG_RECV_H_