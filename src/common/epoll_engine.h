#ifndef KL_COMMON_EPOLL_ENGINE_H_
#define KL_COMMON_EPOLL_ENGINE_H_

#include <sys/epoll.h>
#include <list>
#include "socknotifier.h"

#define KL_COMMON_EPOLL_FD_SIZE		1024    //epoll句柄的默认绑定数

class CEpollEngine : public CSockNotifier
{
public:
    CEpollEngine();
    CEpollEngine(int size, int timeout);

    ~CEpollEngine();

    int attach(CSockObserver *pObserver, uint32_t nstatus);
    int detach(CSockObserver *pObserver);
	void run();
	int set_ob_status(CSockObserver *psock_observer, uint32_t nstatus);
	void Stop();
private:
	//打开epoll库，返回epoll描述符
	int open(int size);

	int m_epfd;
	int m_timeout;
	bool m_stop_flag;
	struct epoll_event m_events[KL_COMMON_EPOLL_FD_SIZE];
	std::list<CSockObserver*> m_ObList;
};
#endif //KL_COMMON_EPOLL_ENGINE_H_
