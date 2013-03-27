#ifndef KL_COMMON_THREAD_H_
#define KL_COMMON_THREAD_H_

#include <pthread.h>
#include "thread_func.h"

typedef pthread_t kl_thread_t;

//the obj of CThread must be created on heap
//and the obj can't be release by call delete
class CThread : public CThreadFunc
{
public:
	CThread();
	/*
	 * @param: pThreadFunc, point at a obj must create by new
	 * @param: stack_size, the size of thread's stack, set default value by use 0
	 * @param: bdetach, whether set detach state
	 */
	CThread(CThreadFunc *pThreadFunc, const int stack_size = 0, bool bdetach = true);
	virtual ~CThread();
	//start thread
	int start();
	kl_thread_t get_thread_id(); 
private:
	static void* thread_entrance(void *pParam);
	int init_pthread_attr(const int stack_size, bool bdetach);

private:
	CThreadFunc *m_pthread_func;
	kl_thread_t m_tid;
	pthread_attr_t m_attr;
};

#endif //KL_COMMON_THREAD_H_