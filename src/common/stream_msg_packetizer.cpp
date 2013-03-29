#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"
#include "msg_queue.h"
#include "common_types.h"
#include "common_protocol.h"
#include "stream_msg_packetizer.h"

CStreamMsgPacketizer::CStreamMsgPacketizer(CMsgQueue **ppmsg_queue, \
	int msg_queue_count) : CSockStreamOB(), m_nbody_recv(0), \
	m_nheader_recv(0), m_pkg_header(), m_pbody(NULL), \
	m_ppstream_msg_queue(ppmsg_queue), m_msg_queue_count(msg_queue_count), \
	m_queue_robin_count(0)
{
}

CStreamMsgPacketizer::CStreamMsgPacketizer(int sock, \
	CMsgQueue **ppmsg_queue, int msg_queue_count) : \
	CSockStreamOB(sock), m_nheader_recv(0), m_nbody_recv(0), \
	m_pkg_header(), m_pbody(NULL), m_ppstream_msg_queue(ppmsg_queue), \
	m_msg_queue_count(msg_queue_count), m_queue_robin_count(0)
{
}

CStreamMsgPacketizer::~CStreamMsgPacketizer()
{
}

void CStreamMsgPacketizer::work(CSockNotifier *psock_notifier, uint32_t nstatus)
{
	int nbytes_recv;
	int64_t nbytes_body;
	byte buf[KL_COMMON_BUF_SIZE];
	byte *pbuf;
	
	if(nstatus & EPOLLIN)
	{
		memset(buf, 0, KL_COMMON_BUF_SIZE);
		nbytes_recv = stream_recv(buf, KL_COMMON_BUF_SIZE);
		if(nbytes_recv <= 0)
		{
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"stream(fd: %d) recv failed, err: %s", \
				__LINE__, get_fd(), strerror(errno));
			psock_notifier->detach(this);
			delete this;
			return;
		}

		pbuf = buf;
		while(nbytes_recv > 0){
			if(m_nheader_recv < sizeof(pkg_header))
			{
				if(nbytes_recv < sizeof(pkg_header) - m_nheader_recv)
				{
					memcpy((void*)(&m_pkg_header + m_nheader_recv), \
						pbuf, nbytes_recv);
					m_nheader_recv += nbytes_recv;
					return;
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
					return;
				}
				memcpy((void*)(&m_pkg_header + m_nbody_recv + 2), pbuf, \
					nbytes_body - m_nbody_recv);
				nbytes_recv -= nbytes_body - m_nbody_recv;
				pbuf += nbytes_body - m_nbody_recv;
				//push to message queue
#ifdef _DEBUG
				printf("file: "__FILE__", line: %d, " \
					"push msg(cmd = %d) to msg queue(id = %d)", \
					__LINE__, m_pkg_header.cmd, m_queue_robin_count);
#endif //_DEBUG
				pkg_message *pkg_msg_ptr = new pkg_message();
				pkg_msg_ptr->msg_stream_ptr = (void*)this;
				pkg_msg_ptr->pkg_len = nbytes_body + 2;
				pkg_msg_ptr->pkg_ptr = m_pbody;
				m_ppstream_msg_queue[m_queue_robin_count++]->push_msg(pkg_msg_ptr);
				m_pbody = NULL;
				m_nbody_recv = 0;
				m_nheader_recv = 0;
				if(m_queue_robin_count == m_msg_queue_count)
					m_queue_robin_count = 0;
				memset(&m_pkg_header, 0, sizeof(pkg_header));
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