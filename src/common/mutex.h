#ifndef KL_COMMON_LOCK_H_
#define KL_COMMON_LOCK_H_

#include <pthread.h>
class CMutex
{
public:
	CMutex();
	virtual ~CMutex();

	int lock();
	int trylock()
	int unlock();
private:
	pthread_mutex_t m_Mutex;
};
#endif //KL_COMMON_LOCK_H_