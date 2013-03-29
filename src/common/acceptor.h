#ifndef KL_COMMON_ACCEPTOR_H_
#define KL_COMMON_ACCEPTOR_H_

#include "sock.h"
#include "sockstream.h"
#include "inetaddr.h"
class CAcceptor : public CSock
{
public:
	/*
	 * @param: bind_port, the listen port
	 * @param: backlog, the maximum connection be listened
	 * @param: timeout, keepalive msg timeout
	 */
	CAcceptor(const char *host, int bind_port, int backlog, int timeout);
	CAcceptor(CInetAddr& sockAddr, int backlog, int timeout);

	int Accept(CSockStream *pSockStream);
protected:
	int setserveropt(int timeout);
	int setkeepalive(int idleSeconds);
};
#endif //KL_COMMON_ACCEPTOR_H_
