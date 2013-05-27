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
	int ret;
	char buf[1024];
	char ip_str[16];
	struct hostent hostinfo, *phostent;
	memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	if((hostname != NULL) && (gethostbyname_r(hostname, &hostinfo, buf, \
		sizeof(buf), &phostent, &ret) == 0))
	{
		inet_aton(inet_ntop(phostent->h_addrtype, phostent->h_addr, ip_str, \
			sizeof(ip_str)), &(m_sockaddr.sin_addr));
	}
	else
	{
		inet_aton("0.0.0.0", &(m_sockaddr.sin_addr));
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
	//KL_SYS_INFOLOG("get ip address: %s", paddress);
	memcpy(buf, paddress, strlen(paddress));
	return 0;
}

int CInetAddr::getport()
{
	return ntohs(m_sockaddr.sin_port);
}

int CInetAddr::get_host_name(char *buf, int size)
{
	int nlen;
	int ret;
	char host_buf[1024];
	struct hostent hostinfo, *phostent;

	memset(buf, 0, size);
	if(gethostbyaddr_r(&m_sockaddr.sin_addr, sizeof(struct in_addr), \
		AF_INET, &hostinfo, host_buf, sizeof(host_buf), &phostent, &ret) == 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call gethostbyaddr_r failed, error code: %d", \
			__LINE__, ret);
		return -1;
	}
	nlen = strlen(phostent->h_name);
	if(nlen >= size)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the buffer size is less than the least space of host name string", \
			__LINE__);
		return -1;
	}
	memcpy(buf, phostent->h_name, nlen);
	return 0;
}
