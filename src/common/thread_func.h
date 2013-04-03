#ifndef KL_COMMON_THREAD_FUNC_H_
#define KL_COMMON_THREAD_FUNC_H_

class CThreadFunc
{
public:
	CThreadFunc() {}
	virtual ~CThreadFunc() {}

	/*
	 * @description: overwrite run function to define the specific
	                 executive function
	 */
	virtual int run() {}
	/*
	 * @description: overwrite this function to stop the thread safely 
	                 if the thread is blocked in a infinite loop
	 */
	virtual int stop() {}
};

#endif //KL_COMMON_HTREAD_FUNC_H_