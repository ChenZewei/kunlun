#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "inetaddr.h"

CInetAddr::CInetAddr(const char *hostname, int port)
{
	setsockaddr(hostname, port);
}

CInetAddr::~CInetAddr()
{
}

CInetAddr& CInetAddr::operator = (const struct sockaddr_in& sockAddr)
{
	memcpy(&m_SockAddr, &sockAddr, sizeof(sockAddr));
	return *this;
}

void CInetAddr::setsockaddr(const char *hostname, int port)
{
	memset(&m_SockAddr, 0, sizeof(m_SockAddr));
	m_SockAddr.sin_family = AF_INET;
	m_SockAddr.sin_port = htons(port);
	struct hostent *phostent;
	if(hostname != NULL && \
			(phostent = gethostbyname(hostname)) != NULL){
		inet_aton(phostent->h_addr, &m_SockAddr.sin_addr);
	}else{
		inet_aton("0.0.0.0", &m_SockAddr.sin_addr);
	}
}

void CInetAddr::setsockaddr(const struct sockaddr_in& sockAddr)
{
	memcpy(&m_SockAddr, &sockAddr, sizeof(sockAddr));
}

struct sockaddr* CInetAddr::getsockaddr()
{
	return (struct sockaddr*)&m_SockAddr;
}

int CInetAddr::getport()
{
	return ntohl(m_SockAddr.sin_port);
}

int CInetAddr::getHostname(char *buf, int size)
{
	int nlen;
	struct hostent *phostent;
	phostent = gethostbyaddr(&m_SockAddr.sin_addr, \
		sizeof(struct in_addr), AF_INET);
	if(phostent != NULL){
		nlen = strlen(phostent->h_name) + 1;
		if(nlen > size){
			return -1;
		}
		memcpy(buf, phostent->h_name, nlen);
		return 0;
	}
	return -1;
}
