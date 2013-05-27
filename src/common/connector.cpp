#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "log.h"
#include "connector.h"

CConnector::CConnector(const char *host, int port, int bport /*= -1*/) : \
	CSock(AF_INET, SOCK_STREAM), m_serveraddr(host, port), m_isconnected(false)
{
	int res;
	if(bport != -1)
	{
		if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, \
			&res, sizeof(int)) < 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"set SO_REUSEADDR failed, err: %s", \
				__LINE__, strerror(errno));
			throw errno;
		}
		CInetAddr bindaddr(NULL, bport);
		if(bind(m_fd, bindaddr.getsockaddr(), sizeof(struct sockaddr)) < 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"bind connector to address(port: %d) failed, err: %s", \
				__LINE__, bport, strerror(errno));
			throw errno;
		}
	}
}

CConnector::CConnector(const CInetAddr& serveraddr, int bport /*= -1*/) : \
	CSock(AF_INET, SOCK_STREAM), m_serveraddr(serveraddr), m_isconnected(false)
{
	int res;
	if(bport != -1)
	{
		if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &res, sizeof(int)) < 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"set SO_REUSEADDR failed, err: %s", \
				__LINE__, strerror(errno));
			throw errno;
		}
		CInetAddr bindaddr(NULL, bport);
		if(bind(m_fd, bindaddr.getsockaddr(), sizeof(struct sockaddr)) < 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"bind connector to address(port: %d) failed, err: %s", \
				__LINE__, bport, strerror(errno));
			throw errno;
		}
	}
}

CConnector::~CConnector()
{
	/* 
	 * if socket is connected, the socket has been pass to sock stream and
	   it will be closed by sock stream, set m_fd to -1 just prevent connector
	   close the socket when connector was deleted, otherwise, the sock stream
	   will be call failed.
	 */
	if(m_isconnected)
		m_fd = -1;
}

int CConnector::stream_connect(CSockStream *psockstream)
{
	int ret;
	if(m_fd == -1)
		return -1;
	if(connect(m_fd, m_serveraddr.getsockaddr(), sizeof(struct sockaddr)) == -1)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"connector connect to server failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}
	if((ret = psockstream->setsockstream(m_fd)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"connector set sock stream failed, err: %s", \
			__LINE__, strerror(ret));
		return -1;
	}
	//connect successfully, connector dispatch sock fd to sock stream
	m_isconnected = true;
	return 0;
}
