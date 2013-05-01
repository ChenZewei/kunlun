#ifndef KL_COMMON_TIMED_STREAM_H_
#define KL_COMMON_TIMED_STREAM_H_
#include "sockstream.h"
class CTimedStream : public CSockStream
{
public:
	CTimedStream();
	CTimedStream(int sock);
	/*
	 * @param: bclosed, sign whether close the socket when a sock stream 
	           obj was deleted.
			   set to false, not to close the socket.
			   set to true, close it.
	 */
	CTimedStream(int sock, bool bclosed);
	~CTimedStream();

	int stream_send(const void *buf, size_t len, int timeout);
	int stream_recv(void *buf, size_t len, int timeout);
};
#endif //KL_COMMON_POLL_STREAM_H_