#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "acceptor.h"
#include "log.h"

CAcceptor::CAcceptor(const char *host, int bind_port, \
	int backlog, int timeout) : \
	CSock(AF_INET, SOCK_STREAM)
{
	CInetAddr sockAddr(host, bind_port);
	int res;

	if (setsockopt(m_fd, SOL_SOCKET, \
		SO_REUSEADDR, &res, sizeof(int)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"set SO_REUSEADDR failed, err: %s", \
			__LINE__, strerror(errno));
	}

	if(bind(m_fd, sockAddr.getsockaddr(), \
		sizeof(struct sockaddr)) == -1){
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call bind failed, err: %s", \
			__LINE__, strerror(errno));
	}
	listen(m_fd, backlog);

	if(timeout > 0) //server option
		setserveropt(timeout);

	setnonblocking();
}

CAcceptor::CAcceptor(CInetAddr& sockAddr, 
	int backlog, int timeout) : CSock(AF_INET, SOCK_STREAM)
{
	int res;
	if (setsockopt(m_fd, SOL_SOCKET, \
		SO_REUSEADDR, &res, sizeof(int)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"set SO_REUSEADDR failed, err: %s", \
			__LINE__, strerror(errno));
	}

	if(bind(m_fd, sockAddr.getsockaddr(), \
		sizeof(struct sockaddr)) == -1){
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"call bind failed, err: %s", \
				__LINE__, strerror(errno));
	}
	listen(m_fd, backlog);

	if(timeout > 0) //server option
		setserveropt(timeout);

	setnonblocking();
}

int CAcceptor::Accept(CSockStream *pSockStream)
{
	int connfd;
	struct sockaddr sockAddr;
	socklen_t len = sizeof(sockAddr);

	connfd = accept(m_fd, &sockAddr, &len);
	if(connfd == -1){
		return -1;
	}
	pSockStream->setsockstream(connfd);
	return 0;
}

int CAcceptor::setserveropt(int timeout)
{
	int flags;
	int result;

	struct linger linger;
	struct timeval waittime;

	linger.l_onoff = 1;
	linger.l_linger = timeout * 100;
	if (setsockopt(m_fd, SOL_SOCKET, SO_LINGER, \
                &linger, (socklen_t)sizeof(struct linger)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : ENOMEM;
	}

	waittime.tv_sec = timeout;
	waittime.tv_usec = 0;

	if (setsockopt(m_fd, SOL_SOCKET, SO_SNDTIMEO, \
		&waittime, (socklen_t)sizeof(struct timeval)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : ENOMEM;
	}

	if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, \
               &waittime, (socklen_t)sizeof(struct timeval)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : ENOMEM;
	}

	flags = 1;
	if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, \
		(char *)&flags, sizeof(flags)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : EINVAL;
	}

	if ((result=setkeepalive(2 * timeout + 1)) != 0)
	{
		return result;
	}

	return 0;
}

int CAcceptor::setkeepalive(int idleSeconds)
{
	int keepAlive;

#ifdef OS_LINUX
	int keepIdle;
	int keepInterval;
	int keepCount;
#endif

	keepAlive = 1;
	if(setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, \
		(char *)&keepAlive, sizeof(keepAlive)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : EINVAL;
	}

#ifdef OS_LINUX
	keepIdle = idleSeconds;
	if (setsockopt(m_fd, SOL_TCP, TCP_KEEPIDLE, (char *)&keepIdle, \
		sizeof(keepIdle)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : EINVAL;
	}

	keepInterval = 10;
	if (setsockopt(m_fd, SOL_TCP, TCP_KEEPINTVL, (char *)&keepInterval, \
		sizeof(keepInterval)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : EINVAL;
	}

	keepCount = 3;
	if (setsockopt(m_fd, SOL_TCP, TCP_KEEPCNT, (char *)&keepCount, \
		sizeof(keepCount)) < 0)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"call setsockopt failed, err: %s", \
			__LINE__, strerror(errno));
		return errno != 0 ? errno : EINVAL;
	}
#endif

	return 0;
}