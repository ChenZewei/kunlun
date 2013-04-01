#ifndef KL_COMMON_SOCK_OBSERVER_H_
#define KL_COMMON_SOCK_OBSERVER_H_
class CSockNotifier;
#include "socknotifier.h"

class CSockObserver
{
public:
    virtual ~CSockObserver();

    virtual void work(CSockNotifier *psock_notifier, uint32_t nstatus) = 0;
    virtual int get_fd() const = 0;
protected:
    CSockObserver();
};
#endif //KL_COMMON_SOCK_OBSERVER_H_
