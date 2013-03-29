#ifndef KL_COMMON_BASE_SERVER_H_
#define KL_COMMON_BASE_SERVER_H_
class CMsgQueue;
class CBaseServer
{
public:
	CBaseServer();
	~CBaseServer();

	int run();
protected:
	int m_work_thread_count;
	int m_msg_queue_count;
	CMsgQueue **m_ppmsg_queue;
};
#endif //KL_COMMON_BASE_SERVER_H_