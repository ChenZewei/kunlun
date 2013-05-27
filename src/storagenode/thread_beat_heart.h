#ifndef KL_STORAGE_THREAD_BEAT_HEART_H_
#define KL_STORAGE_THREAD_BEAT_HEART_H_

#include "inetaddr.h"
#include "thread_func.h"
#include "common_protocol.h"
class CMsgQueue;
class CTimedStream;
class CThreadBeatHeart : public CThreadFunc
{
public:
	CThreadBeatHeart(CInetAddr proxy_addr, CMsgQueue *psync_msg_queue, \
		int nthread_index);
	~CThreadBeatHeart();

	int run();
	int stop();
private:
	/*
	 * @brief: send a joining message to every proxy node
	 * @param: pjoin_stream, the stream to send storage node joining msg
	 */
	int join_and_report(CTimedStream *pjoin_stream);
	int report_vnode_info(CTimedStream *preport_stream);
	int report_sync_down_info(CTimedStream *preport_stream);
	int merge_vnode_status(int vnode_id, byte vnode_status);

	bool m_bstop_flag;
	int  m_nthread_index;
	CInetAddr m_proxy_addr;
	CMsgQueue *m_psync_msg_queue;
};
#endif //KL_STORAGE_THREAD_BEAT_HEART_H_