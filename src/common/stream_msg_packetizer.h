#ifndef KL_COMMON_STREAM_MSG_PACKETIZER_H_
#define KL_COMMON_STREAM_MSG_PACKETIZER_H_
#include <stdint.h>
#include "sockstreamOB.h"
#include "common_protocol.h"
class CMsgQueueArr;
/*
 * @description: unfortunately, TCP is a stream protocol, we can't know where is the
                 end of a message package, so we need a msg packetizer to pack the msg
 */
class CStreamMsgPacketizer : public CSockStreamOB
{
public:
	CStreamMsgPacketizer(CMsgQueueArr *msg_queue_arr_ptr);
	CStreamMsgPacketizer(int sock, CMsgQueueArr *msg_queue_arr_ptr);
	~CStreamMsgPacketizer();
	/*
	 * @description: work function receive data when epoll notify it that the buffer of 
	                 the sock stream has data and it pack the data to a intact msg pkg
					 and push the msg pkg to a msg queue chosen from all msg queues.
					 a msg queue is chosen through the mode of round robin.
					 if a sock stream is closed by the peer side, work function must detach
					 the sock stream object with epoll engine and delete the sock stream object
	 */
	void work(CSockNotifier *psock_notifier, uint32_t nstatus);
private:
	int64_t m_nheader_recv; //bytes of msg header that has been received
	int64_t m_nbody_recv; //bytes of msg body that has been received
	pkg_header m_pkg_header;
	byte *m_pbody;
	CMsgQueueArr *m_pmsg_queue_arr;
};

#endif //KL_COMMON_STREAM_MSG_PACKETIZER_H_