#include <new>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "log.h"
#include "common_types.h"
#include "msg_queue.h"
#include "acceptorOB.h"
#include "stream_msg_packetizer.h"

CAcceptorOB::CAcceptorOB(const char *host, int bind_port, \
	int backlog, int timeout, CMsgQueueArr *msg_queue_arr_ptr) : \
	CAcceptor(host, bind_port, backlog, timeout), \
	m_pmsg_queue_arr(msg_queue_arr_ptr)
{
}

CAcceptorOB::CAcceptorOB(CInetAddr& sockAddr, int backlog, \
	int timeout,CMsgQueueArr *msg_queue_arr_ptr) : \
	CAcceptor(sockAddr, backlog, timeout), \
	m_pmsg_queue_arr(msg_queue_arr_ptr)
{
}

int CAcceptorOB::get_fd() const
{
	return getsocket();
}

void CAcceptorOB::work(CSockNotifier *psock_notifier, uint32_t nstatus)
{
	int res;
	CSockStreamOB *psock_stream_ob;
	if(nstatus & KL_COMMON_STREAM_IN)
	{
		while(true)
		{
			try
			{
				psock_stream_ob = new CStreamMsgPacketizer(m_pmsg_queue_arr);
			}
			catch(std::bad_alloc)
			{
				psock_stream_ob = NULL;
			}
			catch(int errcode)
			{
				KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
					"call CStreamMsgPacketizer constructor failed, err: %s", \
					__LINE__, strerror(errcode));
				continue;
			}
			if(psock_stream_ob == NULL)
			{
				KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
					"no more memory to ceate sock stream");
				continue;
			}
			res = stream_accept(psock_stream_ob);
			if(res == -1)
			{	//failed or has no connection
				delete psock_stream_ob;
				if(errno == EINTR)
					continue;
				break;
			}
			if(psock_notifier->attach(psock_stream_ob, \
				KL_COMMON_STREAM_IN | KL_COMMON_STREAM_EXTEND) != 0)
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
