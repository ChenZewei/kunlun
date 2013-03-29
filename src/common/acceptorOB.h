#ifndef KL_COMMON_ACCEPTOR_OB_H_
#define KL_COMMON_ACCEPTOR_OB_H_
#include "acceptor.h"
#include "sockobserver.h"
#include "inetaddr.h"
class CMsgQueue;
class CAcceptorOB : public CAcceptor, public CSockObserver
{
public:
	/*
	 * @param: ppmsg_queue, used to deliver messages by sockstream
	 * @param: msg_queue_count, msg queue size
	 */
	CAcceptorOB(const char *host, int bind_port, \
		int backlog, int timeout, CMsgQueue **ppmsg_queue, \
		int msg_queue_count);
	CAcceptorOB(CInetAddr& sockAddr, int backlog, \
		int timeout, CMsgQueue **ppmsg_queue, int msg_queue_count);

	void work(CSockNotifier *psock_notifier, \
		uint32_t nstatus);
	int get_fd() const;
private:
	CMsgQueue **m_ppmgs_queue;
	int m_msg_queue_count;
};
#endif //_ACCEPTOR_OB_H_
