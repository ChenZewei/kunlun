#ifndef KL_COMMON_MSG_QUEUE_H_
#define KL_COMMON_MSG_QUEUE_H_

#include <queue>
#include "common_protocol.h"
class CMutex;
class CEvent;
class CMsgQueue
{
public:
	CMsgQueue();
	~CMsgQueue();

	void push_msg(pkg_message *pkg_msg_ptr);
	pkg_message* get_msg();
private:
	CMsgQueue(const CMsgQueue&);
	CMsgQueue& operator=(const CMsgQueue&);

	void push(pkg_message *pkg_msg_ptr);
	pkg_message* pop();
private:
	CMutex *m_ppush_mutex;
	CMutex *m_ppop_mutex;
	CEvent *m_pmsg_event;
	std::queue<pkg_message*> m_msg_queue;
};

#endif //KL_COMMON_MSG_QUEUE_H_