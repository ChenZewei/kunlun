#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "thread.h"
#include "log.h"
#include "common_types.h"

CThread::CThread() : m_pthread_func(this), m_tid(-1)
{
#ifdef _DEBUG
	printf("init thread obj\n");
#endif //_DEBUG
	if(init_pthread_attr(0, true) != 0){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"init thread attribute failed", \
			__LINE__);
	}
}

CThread::CThread(CThreadFunc *pThreadFunc, const int stack_size, \
	bool bdetach) : m_pthread_func(pThreadFunc), m_tid(-1)
{
#ifdef _DEBUG
	printf("init thread obj\n");
#endif //_DEBUG
	if(init_pthread_attr(stack_size, bdetach) != 0){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"init thread attribute failed", \
			__LINE__);
	}
}

CThread::~CThread()
{
#ifdef _DEBUG
	printf("delete the thread\n");
#endif //_DEBUG
	if(m_pthread_func != NULL){
		if(m_pthread_func != this)
			delete m_pthread_func;
		m_pthread_func = NULL;
	}
	m_tid = -1;
	pthread_attr_destroy(&m_attr);
}

void* CThread::thread_entrance(void *pParam)
{
	int res = KL_COMMON_EXIT_SYS;
	CThread *pthread = (CThread*)pParam;
	CThreadFunc *pthread_func = pthread->m_pthread_func;

	res = pthread_func->run();
	delete pthread; //thread resource should be release after exit

	return (void*)res;
}

kl_thread_t CThread::get_thread_id()
{
	return m_tid;
}

int CThread::init_pthread_attr(const int stack_size, \
	bool bdetach)
{
	size_t old_stack_size;
	size_t new_stack_size;
	int result;
	int detach_stat;

	if ((result = pthread_attr_init(&m_attr)) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call pthread_attr_init failed, " \
			"err: %s", \
			__LINE__, strerror(result));
		return result;
	}

	if ((result = pthread_attr_getstacksize(&m_attr, &old_stack_size)) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call pthread_attr_getstacksize failed, err: %s", \
			__LINE__, strerror(result));
		return result;
	}

	if (stack_size > 0)
	{
		if (old_stack_size != stack_size)
		{
			new_stack_size = stack_size;
		}
		else
		{
			new_stack_size = 0;
		}
	}
	else if (old_stack_size < 1 * 1024 * 1024)
	{
		new_stack_size = 1 * 1024 * 1024;
	}
	else
	{
		new_stack_size = 0;
	}

	if (new_stack_size > 0)
	{
		if ((result = pthread_attr_setstacksize(&m_attr, \
			new_stack_size)) != 0)
		{
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"call pthread_attr_setstacksize failed, err: %s", \
				__LINE__, strerror(result));
			return result;
		}
	}

	if(bdetach){
		detach_stat = PTHREAD_CREATE_DETACHED;
	}
	detach_stat = PTHREAD_CREATE_JOINABLE;

	if ((result = pthread_attr_setdetachstate(&m_attr, \
		detach_stat)) != 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call pthread_attr_setdetachstate failed, err: %s", \
			__LINE__, strerror(result));
		return result;
	}

	if((result = pthread_attr_setscope(&m_attr, \
		PTHREAD_SCOPE_SYSTEM)) != 0){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"set thread to system level failed, err: %s", \
			__LINE__, strerror(result));
		return result;
	}
	
	return 0;
}

int CThread::start()
{
	int res;
	res = pthread_create(&m_tid, &m_attr, thread_entrance, this);
	if(res != 0){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"create POSIX thread failed, err: %s", \
			__LINE__, strerror(res));
	}
	return res;
}