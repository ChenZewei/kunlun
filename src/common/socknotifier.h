/* 
 * 该类用于封装通知者的抽象，
 * 纯虚类供真实的通知者继承
 */
#ifndef KL_COMMON_SOCK_NOTIFIER_H_
#define KL_COMMON_SOCK_NOTIFIER_H_

class CSockObserver;
#include <stdint.h>
#include "sockobserver.h"

class CSockNotifier
{
public:
    virtual ~CSockNotifier();

    //将观察者和通知者以某种状态绑定在一起
    //由于绑定的形式可能根据具体通知者和观察者的
    //数据结构而各不相同，所以在此定义为纯虚函数
    //供子类实现
    virtual int attach(CSockObserver *psock_observer, uint32_t nStatus) = 0;
    //将已绑定的通知者和观察者解除绑定
    virtual int detach(CSockObserver *psock_observer) = 0;
	//通知者的启动函数同时也是通知函数，
	//用于将自己的状态变化通知给相关的观察者
    virtual void run() = 0;
	virtual int set_ob_status(CSockObserver *pObserver, uint32_t nStatus) = 0;
protected:
    CSockNotifier();
};

#endif //KL_COMMON_SOCK_NOTIFIER_H_
