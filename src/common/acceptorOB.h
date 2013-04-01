#ifndef KL_COMMON_ACCEPTOR_OB_H_
#define KL_COMMON_ACCEPTOR_OB_H_
#include "acceptor.h"
#include "sockobserver.h"
#include "inetaddr.h"
class CMsgQueueArr;
class CAcceptorOB : public CAcceptor, public CSockObserver
{
public:
	/*
	 * @param: ppmsg_queue, used to deliver messages by sockstream
	 * @param: msg_queue_count, msg queue size
	 */
	CAcceptorOB(const char *host, int bind_port, \
		int backlog, int timeout, CMsgQueueArr *msg_queue_arr_ptr);
	CAcceptorOB(CInetAddr& sockAddr, int backlog, \
		int timeout, CMsgQueueArr *msg_queue_arr_ptr);
	/*
	 * @description: work function accept connection requested by connector 
	                 and attach the sock stream corresponding with
					 the connection with epoll engine
	 */
	void work(CSockNotifier *psock_notifier, \
		uint32_t nstatus);
	int get_fd() const;
private:
	CMsgQueueArr *m_pmsg_queue_arr;
};
#endif //_ACCEPTOR_OB_H_
