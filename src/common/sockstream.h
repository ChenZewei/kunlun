#ifndef KL_COMMON_SOCK_STREAM_H_
#define KL_COMMON_SOCK_STREAM_H_

#include "sock.h"

class CSockStream : public CSock
{
public:
	CSockStream();
	CSockStream(int sock);
	
	void setsockstream(int sock);
	int stream_send(const void *buf, size_t len);
	int stream_recv(void *buf, size_t len);
};
#endif //KL_COMMON_SOCK_STREAM_H_
