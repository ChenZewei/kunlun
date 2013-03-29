#ifndef KL_COMMON_LOCK_H_
#define KL_COMMON_LOCK_H_

#include <pthread.h>
class CMutex
{
public:
	friend class CCond;

	CMutex();
	virtual ~CMutex();

	int lock();
	int trylock();
	int unlock();
private:
	CMutex(const CMutex&);
	CMutex& operator=(const CMutex&);

	pthread_mutex_t m_mutex;
};
#endif //KL_COMMON_LOCK_H_