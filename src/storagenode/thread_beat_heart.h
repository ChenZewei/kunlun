#ifndef KL_STORAGE_THREAD_BEAT_HEART_H_
#define KL_STORAGE_THREAD_BEAT_HEART_H_

#include "inetaddr.h"
#include "thread_func.h"
class CTimedStream;
class CThreadBeatHeart : public CThreadFunc
{
public:
	CThreadBeatHeart(CInetAddr proxy_addr);
	~CThreadBeatHeart();

	int run();
	int stop();
private:
	int report_vnode_info(CTimedStream *preport_stream);
	bool m_stop_flag;
	CInetAddr m_proxy_addr;
};
#endif //KL_STORAGE_THREAD_BEAT_HEART_H_