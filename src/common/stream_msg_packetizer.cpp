#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"
#include "msg_queue.h"
#include "common_types.h"
#include "common_protocol.h"
#include "stream_msg_packetizer.h"
#ifdef USE_SELECT
#define KL_COMMON_STREAM_IN
#define KL_COMMON_STREAM_OUT
#else
#ifdef USE_POLL
#define KL_COMMON_STREAM_IN
#define KL_COMMON_STREAM_OUT
#else //default : USE_EPOLL
#include <sys/epoll.h>
#define KL_COMMON_STREAM_IN EPOLLIN
#define KL_COMMON_STREAM_OUT EPOLLOUT
#endif //USE_POLL
#endif //USE_SELECT

CStreamMsgPacketizer::CStreamMsgPacketizer(CMsgQueueArr *msg_queue_arr_ptr) : \
	CSockStreamOB(), m_nbody_recv(0), m_nheader_recv(0), m_pkg_header(), \
	m_pbody(NULL), m_pmsg_queue_arr(msg_queue_arr_ptr)
{
}

CStreamMsgPacketizer::CStreamMsgPacketizer(int sock, \
	CMsgQueueArr *msg_queue_arr_ptr) : \
	CSockStreamOB(sock), m_nheader_recv(0), m_nbody_recv(0), \
	m_pkg_header(), m_pbody(NULL), m_pmsg_queue_arr(msg_queue_arr_ptr)
{
}

CStreamMsgPacketizer::~CStreamMsgPacketizer()
{
	if(m_pbody != NULL)
	{
		delete m_pbody;
		m_pbody = NULL;
	}
}

void CStreamMsgPacketizer::work(CSockNotifier *psock_notifier, uint32_t nstatus)
{
	/*
	 * sock stream have two kinds of circumstance to receive zero byte data.
	 * the first kind: socknotifier notify sock stream to receive data, but there is no
					   data in buffer, and operate system return an error: EAGAIN, that 
					   means the sock stream is closed by the peer side, so we must delete
					   the sock stream obj
	 * the second kind: the sock stream has read all data in buffer, but it try to read again,
	                    and then operate system will return an error: EAGAIN, that just means
						no data to read, but the connection isn't closed, so we can't delete 
						the sock stream obj
	 */
	bool bfirst_recv;
	int nbytes_recv;
	int64_t nbytes_body;
	byte buf[KL_COMMON_BUF_SIZE];
	byte *pbuf;
	
	if(nstatus & KL_COMMON_STREAM_IN)
	{
		bfirst_recv = true;
		while(true)
		{
			memset(buf, 0, KL_COMMON_BUF_SIZE);
			nbytes_recv = stream_recv(buf, KL_COMMON_BUF_SIZE);
			if(nbytes_recv <= 0)
			{
				if(EAGAIN != errno)
				{
					KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
						"stream(fd: %d) recv failed, err: %s", \
						__LINE__, get_fd(), strerror(errno));
				}

				if(bfirst_recv)
				{
					psock_notifier->detach(this);
					delete this;
				}
				return;
			}

			bfirst_recv = false;
			pbuf = buf;
			while(nbytes_recv > 0)
			{
				if(m_nheader_recv < sizeof(pkg_header))
				{
					if(nbytes_recv < sizeof(pkg_header) - m_nheader_recv)
					{
						memcpy((void*)(&m_pkg_header + m_nheader_recv), \
							pbuf, nbytes_recv);
						m_nheader_recv += nbytes_recv;
						break;
					}
					memcpy((void*)(&m_pkg_header + m_nheader_recv), pbuf, \
						sizeof(pkg_header) - m_nheader_recv);
					nbytes_recv -= sizeof(pkg_header) - m_nheader_recv;
					pbuf += sizeof(pkg_header) - m_nheader_recv;
					m_nheader_recv = sizeof(pkg_header);
				}

				nbytes_body = CSERIALIZER::buff2int64(m_pkg_header.pkg_len);
				if(m_pbody == NULL)
				{
					m_pbody = new byte[nbytes_body + 2];
					*m_pbody = m_pkg_header.cmd;
					*(m_pbody + 1) = m_pkg_header.status;
				}
				if(m_nbody_recv < nbytes_body)
				{
					if(nbytes_recv < nbytes_body - m_nbody_recv)
					{
						memcpy((void*)(m_pbody + m_nbody_recv + 2), pbuf, \
							nbytes_recv);
						m_nbody_recv += nbytes_recv;
						break;
					}
					memcpy((void*)(m_pbody + m_nbody_recv + 2), pbuf, \
						nbytes_body - m_nbody_recv);
					nbytes_recv -= nbytes_body - m_nbody_recv;
					pbuf += nbytes_body - m_nbody_recv;
					//push msg pkg to message queue
#ifdef _DEBUG
					KL_SYS_DEBUGLOG("file: "__FILE__", line: %d, " \
						"push msg(cmd = %d) to msg queue(id = %d)", \
						__LINE__, m_pkg_header.cmd, m_pmsg_queue_arr->m_queue_robin);
#endif //_DEBUG
					pkg_message *pkg_msg_ptr = new pkg_message();
					pkg_msg_ptr->msg_stream_ptr = (void*)this;
					pkg_msg_ptr->pkg_len = nbytes_body + 2;
					pkg_msg_ptr->pkg_ptr = m_pbody;
					(m_pmsg_queue_arr->getmsgqueuebyrobin())->push_msg(pkg_msg_ptr);
					m_pbody = NULL;
					m_nbody_recv = 0;
					m_nheader_recv = 0;
					memset(&m_pkg_header, 0, sizeof(pkg_header));
				}
			}
		}
		return;
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"stream(fd: %d) occurred with an undefined status", \
		__LINE__, get_fd());
	psock_notifier->detach(this);
	delete this;
}