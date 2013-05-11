#ifndef KL_STORAGE_THREAD_SYNC_DATA_H_
#define KL_STORAGE_THREAD_SYNC_DATA_H_
#include "thread_func.h"
class CMsgQueue;
class CThreadSyncData : public CThreadFunc
{
public:
	CThreadSyncData(CMsgQueue *psync_msg_queue);
	~CThreadSyncData();

	int run();
	int stop();
private:
	CMsgQueue *m_psync_msg_queue;
};
#endif //KL_STORAGE_THREAD_SYNC_DATA_H_