#ifndef KL_COMMON_SOCK_NOTIFIER_H_
#define KL_COMMON_SOCK_NOTIFIER_H_

class CSockObserver;
#include <stdint.h>
#include "sockobserver.h"

class CSockNotifier
{
public:
    virtual ~CSockNotifier();

	/*
	 * @description: attach function will attach observer with notifier. notifier 
	                 will notify the observer attached with it when the observer's 
					 status is changed
	 */
    virtual int attach(CSockObserver *psock_observer, uint32_t nStatus) = 0;
    virtual int detach(CSockObserver *psock_observer) = 0;
    virtual int notify() = 0;
	virtual int set_ob_status(CSockObserver *pObserver, uint32_t nStatus) = 0;
protected:
    CSockNotifier();
};

#endif //KL_COMMON_SOCK_NOTIFIER_H_
