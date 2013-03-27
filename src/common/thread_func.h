#ifndef KL_COMMON_THREAD_FUNC_H_
#define KL_COMMON_THREAD_FUNC_H_

class CThreadFunc
{
public:
	CThreadFunc() {}
	virtual ~CThreadFunc() {}

	virtual int run() {}
};

#endif //KL_COMMON_HTREAD_FUNC_H_