#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "log.h"
#include "global.h"
#include "msg_queue.h"
#include "acceptorOB.h"
#include "stream_msg_packetizer.h"

CAcceptorOB::CAcceptorOB(const char *host, int bind_port, \
	int backlog, int timeout, CMsgQueue **ppmsg_queue, \
	int msg_queue_count) : CAcceptor(host, bind_port, \
	backlog, timeout), m_ppmgs_queue(ppmsg_queue), \
	m_msg_queue_count(msg_queue_count)
{
}

CAcceptorOB::CAcceptorOB(CInetAddr& sockAddr, int backlog, \
	int timeout, CMsgQueue **ppmsg_queue, int msg_queue_count) : \
	CAcceptor(sockAddr, backlog, timeout), m_ppmgs_queue(ppmsg_queue), \
	m_msg_queue_count(msg_queue_count)
{
}

int CAcceptorOB::get_fd() const
{
	return getSocket();
}

void CAcceptorOB::work(CSockNotifier *psock_notifier, uint32_t nstatus)
{
	int res;
	CSockStreamOB *psock_stream_ob;
	if(nstatus & EPOLLIN){
		while(true){
			psock_stream_ob = new CStreamMsgPacketizer(m_ppmgs_queue, \
				m_msg_queue_count);
			res = Accept(psock_stream_ob);
			if(res == -1){	//failed or has no connect
				delete psock_stream_ob;
				if(errno == EINTR)
					continue;
				break;
			}
			if(psock_notifier->attach(psock_stream_ob, \
				EPOLLIN | EPOLLET) != 0)
			{
				delete psock_stream_ob;
			}
		}
		return;
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"acceptor's status is undefined", __LINE__);
}
