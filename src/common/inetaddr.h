#ifndef KL_COMMON_INET_ADDR_H_
#define KL_COMMON_INET_ADDR_H_

#include <netinet/in.h>
#include <sys/socket.h>

class CInetAddr
{
public:
	CInetAddr();
	CInetAddr(const char *hostname, int port);
	CInetAddr& operator = (const struct sockaddr_in& sockaddr);
	~CInetAddr();

	int get_host_name(char *buf, int size);
	int getipaddress(char *buf, int size);
	struct sockaddr* getsockaddr();
	void setsockaddr(const char *hostname, int port);
	void setsockaddr(const struct sockaddr_in& sockaddr);
	int getport();
private:
	struct sockaddr_in m_sockaddr;
};
#endif //KL_COMMON_INET_ADDR_H_
