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
	if(bport != -1){
		if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, \
			&res, sizeof(int)) < 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"set SO_REUSEADDR failed, err: %s", \
				__LINE__, strerror(errno));
		}
		CInetAddr bindAddr(NULL, bport);
		bind(m_fd, bindAddr.getsockaddr(), \
			sizeof(struct sockaddr));
	}
}

CConnector::CConnector(const CInetAddr& serverAddr, int bport /*= -1*/) : \
	CSock(AF_INET, SOCK_STREAM), m_serveraddr(serverAddr), m_isconnected(false)
{
	int res;
	if(bport != -1){
		if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, \
			&res, sizeof(int)) < 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"set SO_REUSEADDR failed, err: %s", \
				__LINE__, strerror(errno));
		}
		CInetAddr bindAddr(NULL, bport);
		bind(m_fd, bindAddr.getsockaddr(), \
			sizeof(struct sockaddr));
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

int CConnector::stream_connect(CSockStream *pSockStream)
{
	if(m_fd == -1)
		return -1;
	if(connect(m_fd, m_serveraddr.getsockaddr(), \
		sizeof(struct sockaddr)) == -1){
		return -1;
	}
	pSockStream->setsockstream(m_fd);
	//connect successfully, connector dispatch sock fd to sock stream
	m_isconnected = true;
	return 0;
}
