#ifndef KL_COMMON_RWLOCK_H_
#define KL_COMMON_RWLOCK_H_

#include <pthread.h>

class CRWLock
{
public:
	CRWLock();
	virtual ~CRWLock();

	int wrlock();
	int rdlock();
	int trywrlock();
	int tryrdlock();
	int unlock();
private:
	pthread_rwlock_t m_rwlock;
};

#endif //KL_COMMON_RWLCOK_H_