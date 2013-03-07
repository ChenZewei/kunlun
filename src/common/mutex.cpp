#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "mutex.h"
#include "common_types.h"

CMutex::CMutex()
{
	m_pMutex = new pthread_mutex_t;
	m_pMattr = new pthread_mutexattr_t;

	if(m_pMutex == NULL){
		errout("new pthread_mutex_t failed");
	}

	if(m_pMattr == NULL){
		errout("new pthread_mutexattr_t failed");
	}

	if(pthread_mutexattr_init(m_pMattr) == -1){
		errout("pthread_mutexattr_init failed");
	}

	if(pthread_mutex_init(m_pMutex, m_pMattr) == -1){
		errout("pthread_mutex_init failed");
	}

	delete m_pMattr;
}

CMutex::~CMutex()
{
	if(m_pMattr != NULL){
		delete m_pMattr;
	}
	if(m_pMutex != NULL){
		delete m_pMutex;
	}
}

void CMutex::Lock()
{
	pthread_mutex_lock(m_pMutex);
}

void CMutex::Unlock()
{
	pthread_mutex_unlock(m_pMutex);
}