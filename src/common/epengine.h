#ifndef KL_COMMON_EPOLL_ENGINE_H_
#define KL_COMMON_EPOLL_ENGINE_H_

#include <sys/epoll.h>
#include <list>
#include "subject.h"

#define EPOLL_FD_SIZE   256     //epoll句柄的默认绑定数

class CEpEngine : public CSubject
{
public:
    CEpEngine();
    CEpEngine(int size, int timeout);

    ~CEpEngine();

    void Attach(CObserver *pObserver, uint32_t nstatus);
    void Detach(CObserver *pObserver);
	void Run();
	void SetOBStatus(CObserver *pObserver, uint32_t nstatus);
	void Stop();
private:
	//打开epoll库，返回epoll描述符
	int open(int size);

	int m_epfd;
	int m_timeout;
	bool m_stop_flag;
	struct epoll_event m_events[EPOLL_FD_SIZE];
	list<CObserver*> m_ObList;
};
#endif //KL_COMMON_EPOLL_ENGINE_H_
