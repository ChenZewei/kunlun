#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "mutex.h"

CMutex::CMutex()
{
	m_pMutex = new pthread_mutex_t;
	m_pMattr = new pthread_mutexattr_t;

	if(m_pMutex == NULL || m_pMattr == NULL){
		perror("new pthread_mutex_t or pthread_mutexattr_t failed");
	}else{
		if(pthread_mutexattr_init(m_pMattr) == -1){
			perror("pthread_mutexattr_init failed");
		}else{
			if(pthread_mutex_init(m_pMutex, m_pMattr) == -1){
				perror("pthread_mutex_init failed");
			}
		}
	}
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