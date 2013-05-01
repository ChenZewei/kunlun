#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "log.h"
#include "inetaddr.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CInetAddr::CInetAddr()
{
	memset(&m_sockaddr, 0, sizeof(m_sockaddr));
}

CInetAddr::CInetAddr(const char *hostname, int port)
{
	setsockaddr(hostname, port);
}

CInetAddr::~CInetAddr()
{
}

CInetAddr& CInetAddr::operator = (const struct sockaddr_in& sockaddr)
{
	memcpy(&m_sockaddr, &sockaddr, sizeof(sockaddr));
	return *this;
}

void CInetAddr::setsockaddr(const char *hostname, int port)
{
	memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	struct hostent *phostent;
	if(hostname != NULL && (phostent = gethostbyname(hostname)) != NULL)
	{
		inet_aton(phostent->h_addr, &m_sockaddr.sin_addr);
	}
	else
	{
		inet_aton("0.0.0.0", &m_sockaddr.sin_addr);
	}
}

void CInetAddr::setsockaddr(const struct sockaddr_in& sockaddr)
{
	memcpy(&m_sockaddr, &sockaddr, sizeof(sockaddr));
}

struct sockaddr* CInetAddr::getsockaddr()
{
	return (struct sockaddr*)&m_sockaddr;
}

int CInetAddr::getipaddress(char *buf, int size)
{
#ifdef _DEBUG
	assert(buf && size > 0);
#endif //_DEBUG
	char *paddress;

	memset(buf, 0, size);
	paddress = inet_ntoa(m_sockaddr.sin_addr);
	if(paddress == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"swap ip address to string format failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}
	if(size < strlen(paddress))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the buffer size is less than the least space of ip string", \
			__LINE__);
		return -1;
	}
	memcpy(buf, paddress, strlen(paddress));
	return 0;
}

int CInetAddr::getport()
{
	return ntohl(m_sockaddr.sin_port);
}

int CInetAddr::get_host_name(char *buf, int size)
{
	int nlen;
	struct hostent *phostent;
	phostent = gethostbyaddr(&m_sockaddr.sin_addr, sizeof(struct in_addr), AF_INET);
	if(phostent != NULL)
	{
		nlen = strlen(phostent->h_name) + 1;
		if(nlen > size)
		{
			return -1;
		}
		memcpy(buf, phostent->h_name, nlen);
		return 0;
	}
	return -1;
}
