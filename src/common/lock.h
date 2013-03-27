#ifndef KL_COMMON_LOCK_H_
#define KL_COMMON_LOCK_H_

#include <pthread.h>
class CLock
{
public:
	CLock();
	~CLock();

	int Lock();
	int Unlock();
private:
	pthread_mutex_t m_Mutex;
};
#endif //KL_COMMON_LOCK_H_