#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "sock.h"
#include "log.h"
#include "global.h"

CSock::CSock()
{
	m_fd = -1;
}

CSock::CSock(int domain, int type)
{
	open(domain, type);
}

CSock::~CSock()
{
	if(m_fd != -1){
		close(m_fd);
	}
}

int CSock::open(int domain, int type)
{
	m_fd = socket(domain, type, 0);
	if(m_fd == -1){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"create socket failed, err: %s", \
			__LINE__, strerror(errno));
	}
	return m_fd;
}

int CSock::getSocket() const
{
	return m_fd;
}

int CSock::getLocalAddr(CInetAddr *paddr)
{
	struct sockaddr_in sockAddr;
	socklen_t nlen = sizeof(sockAddr);

	if(paddr == NULL)
		return -1;

	if(getsockname(m_fd, (struct sockaddr*)&sockAddr, \
		&nlen) == -1){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"get local address failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}

	paddr->setsockaddr(sockAddr);
	return 0;
}

int CSock::getPeerAddr(CInetAddr *paddr)
{
	struct sockaddr_in sockAddr;
	socklen_t nlen = sizeof(sockAddr);

	if(paddr == NULL)
		return -1;

	if(getpeername(m_fd, (struct sockaddr*)&sockAddr, \
		&nlen) == -1){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"get peer address failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}

	paddr->setsockaddr(sockAddr);
	return 0;
}

void CSock::setnonblocking()
{
	int opt;

	opt = fcntl(m_fd, F_GETFL);
	if(opt != -1){
		opt = opt | O_NONBLOCK;
		fcntl(m_fd, F_SETFL, opt);
	}
}
