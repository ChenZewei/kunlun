#ifndef KL_COMMON_MSG_LOOPER_H_
#define KL_COMMON_MSG_LOOPER_H_
#include "thread_func.h"
class CMsgQueue;
class CMsgManager;
class CMsgLooper : public CThreadFunc
{
public:
	/*
	 * @param: pmsg_queue, msg looper get msg from the queue
	 * @param: pmsg_manager, register msg and despatch msg,
				must be created on heap and deleted by msg looper
	 */
	CMsgLooper(CMsgQueue *pmsg_queue, CMsgManager *pmsg_manager);
	~CMsgLooper();

	virtual int run();
private:
	CMsgQueue *m_pmsg_queue;
	CMsgManager *m_pmsg_manager;
};

#endif //KL_COMMON_MSG_LOOPER_H_