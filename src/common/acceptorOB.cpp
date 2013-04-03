#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "log.h"
#include "msg_queue.h"
#include "acceptorOB.h"
#include "stream_msg_packetizer.h"

CAcceptorOB::CAcceptorOB(const char *host, int bind_port, \
	int backlog, int timeout, CMsgQueueArr *msg_queue_arr_ptr) : \
	CAcceptor(host, bind_port, backlog, timeout), \
	m_pmsg_queue_arr(msg_queue_arr_ptr)
{
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("CAcceptorOB constructor call successfully");
#endif //_DEBUG
}

CAcceptorOB::CAcceptorOB(CInetAddr& sockAddr, int backlog, \
	int timeout,CMsgQueueArr *msg_queue_arr_ptr) : \
	CAcceptor(sockAddr, backlog, timeout), \
	m_pmsg_queue_arr(msg_queue_arr_ptr)
{
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("CAcceptorOB constructor call successfully");
#endif //_DEBUG
}

int CAcceptorOB::get_fd() const
{
	return getsocket();
}

void CAcceptorOB::work(CSockNotifier *psock_notifier, uint32_t nstatus)
{
	int res;
	CSockStreamOB *psock_stream_ob;
	if(nstatus & EPOLLIN){
		while(true){
			psock_stream_ob = new CStreamMsgPacketizer(m_pmsg_queue_arr);
			res = stream_accept(psock_stream_ob);
			if(res == -1){	//failed or has no connection
				delete psock_stream_ob;
				if(errno == EINTR)
					continue;
				break;
			}
			if(psock_notifier->attach(psock_stream_ob, \
				EPOLLIN | EPOLLET) != 0)
			{
				delete psock_stream_ob;
				psock_stream_ob = NULL;
			}
		}
		return;
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"acceptor's status is undefined", __LINE__);
}
