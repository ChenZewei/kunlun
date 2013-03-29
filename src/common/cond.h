#ifndef KL_COMMON_COND_H_
#define KL_COMMON_COND_H_

#include "mutex.h"
class CCond
{
public:
	CCond();
	virtual ~CCond();

	int wait(CMutex *mutex_ptr);
	int timed_wait(CMutex *mutex_ptr, int timeout);
	int wakeup();
	int wakeup_all();
private:
	CCond(const CCond&);
	CCond& operator=(const CCond&);

	pthread_conde_t m_cond;
};

#endif //KL_COMMON_COND_H_