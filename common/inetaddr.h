#ifndef _INET_ADDR_H_
#define _INET_ADDR_H_

#include <netinet/in.h>
#include <sys/socket.h>

class CInetAddr
{
public:
	CInetAddr(const char *hostname, int port);
	CInetAddr& operator = (const struct sockaddr_in& sockAddr);
	~CInetAddr();

	int GetHostName(char *buf, int size);
	struct sockaddr* GetSockAddr();
	void SetSockAddr(const char *hostname, int port);
	void SetSockAddr(const struct sockaddr_in& sockAddr);
	int GetPort();
private:
	struct sockaddr_in m_SockAddr;
};
#endif //_INET_ADDR_H_
