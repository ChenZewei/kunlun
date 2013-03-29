#ifndef KL_COMMON_STREAM_MSG_PACKETIZER_H_
#define KL_COMMON_STREAM_MSG_PACKETIZER_H_
#include <stdint.h>
#include "sockstreamOB.h"
#include "common_protocol.h"
class CMsgQueue;
class CStreamMsgPacketizer : public CSockStreamOB
{
public:
	CStreamMsgPacketizer(CMsgQueue **ppmsg_queue, int msg_queue_count);
	CStreamMsgPacketizer(int sock, CMsgQueue **ppmsg_queue, int msg_queue_count);
	~CStreamMsgPacketizer();

	void work(CSockNotifier *psock_notifier, uint32_t nstatus);
private:
	int64_t m_nheader_recv; //bytes of msg header that has been received
	int64_t m_nbody_recv; //bytes of msg body that has been received
	int m_msg_queue_count;
	int m_queue_robin_count;
	pkg_header m_pkg_header;
	byte *m_pbody;
	CMsgQueue **m_ppstream_msg_queue;
};

#endif //KL_COMMON_STREAM_MSG_PACKETIZER_H_