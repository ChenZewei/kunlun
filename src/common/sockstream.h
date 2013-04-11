#ifndef KL_COMMON_SOCK_STREAM_H_
#define KL_COMMON_SOCK_STREAM_H_

#include "sock.h"
class CSockStream : public CSock
{
public:
	CSockStream();
	CSockStream(int sock);
	/*
	 * @param: bclosed, sign whether close the socket when a sock stream 
	           obj was deleted.
			   set to false, not to close the socket.
			   set to true, close it.
	 */
	CSockStream(int sock, bool bclosed);
	virtual ~CSockStream();

	/*
	 * @description: use a socket to initilize the sock stream obj
	 */
	void setsockstream(int sock);
	int stream_send(const void *buf, size_t len);
	int stream_recv(void *buf, size_t len);
protected:
	bool m_bclosed;
};
#endif //KL_COMMON_SOCK_STREAM_H_
