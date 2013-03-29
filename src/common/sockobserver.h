#ifndef KL_COMMON_SOCK_OBSERVER_H_
#define KL_COMMON_SOCK_OBSERVER_H_
class CSockNotifier;
#include "socknotifier.h"

class CSockObserver
{
public:
    virtual ~CSockObserver();

    //work函数是每个观察者对外的接口，
    //用于被通知者调用，实现通知者和观察者的数据交互
    virtual void work(CSockNotifier *psock_notifier, uint32_t nstatus) = 0;
    //获取观察者的文件描述符或者套接字描述符
    virtual int get_fd() const = 0;
protected:
    CSockObserver();
};
#endif //KL_COMMON_SOCK_OBSERVER_H_
