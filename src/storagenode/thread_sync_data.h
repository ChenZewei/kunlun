#ifndef KL_STORAGE_THREAD_SYNC_DATA_H_
#define KL_STORAGE_THREAD_SYNC_DATA_H_
#include "inetaddr.h"
#include "thread_func.h"
class CMsgQueue;
class CSockStream;
class storage_sync_event;
class CThreadSyncData : public CThreadFunc
{
public:
	CThreadSyncData(CMsgQueue *psync_msg_queue, CInetAddr master_addr);
	~CThreadSyncData();

	int run();
	int stop();
private:
	int do_sync_event(storage_sync_event *pstorage_sync_event);
	int check_and_sync_vnode(int vnode_id, CTimedStream *psync_stream);
	int sync_vnode_file(const char *path, CTimedStream *psync_stream);

	bool m_bstop_flag;
	CInetAddr m_master_addr;
	CMsgQueue *m_psync_msg_queue;
};
#endif //KL_STORAGE_THREAD_SYNC_DATA_H_