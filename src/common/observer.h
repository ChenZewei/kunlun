#ifndef _OBSERVER_H_
#define _OBSERVER_H_
class CSubject;
#include "subject.h"

class CObserver
{
public:
    virtual ~CObserver();

    //work函数是每个观察者对外的接口，
    //用于被通知者调用，实现通知者和观察者的数据交互
    virtual void Work(CSubject *pSubject, uint32_t nstatus) = 0;
    //获取观察者的文件描述符或者套接字描述符
    virtual int GetFd() const = 0;
protected:
    CObserver();
};
#endif //_OBSERVER_H_
