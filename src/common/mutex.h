#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>
class CMutex
{
public:
	CMutex();
	~CMutex();

	void Lock();
	void Unlock();
private:
	pthread_mutex_t *m_pMutex;
	pthread_mutexattr_t *m_pMattr;
};
#endif //MUTEX_H_