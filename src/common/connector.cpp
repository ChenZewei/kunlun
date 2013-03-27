/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月14日 星期二 16时54分23秒
* File Name: connector.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "connector.h"

CConnector::CConnector(const char *host, int port, int bport /*= -1*/) : \
				CSock(AF_INET, SOCK_STREAM), m_serverAddr(host, port)
{
	if(bport != -1){
		CInetAddr bindAddr(NULL, bport);
		bind(m_fd, bindAddr.getsockaddr(), sizeof(struct sockaddr));
	}
	setNonBlocking();
}

CConnector::CConnector(const CInetAddr& serverAddr, int bport /*= -1*/) : \
				CSock(AF_INET, SOCK_STREAM), m_serverAddr(serverAddr)
{
	if(bport != -1){
		CInetAddr bindAddr(NULL, bport);
		bind(m_fd, bindAddr.getsockaddr(), sizeof(struct sockaddr));
	}
	setNonBlocking();
}

int CConnector::Connect(CSockStream *pSockStream)
{
	if(m_fd == -1)
		return -1;
	if(connect(m_fd, m_serverAddr.getsockaddr(), sizeof(struct sockaddr)) \
			== -1){
		return -1;
	}
	pSockStream->SetSockStream(m_fd);
	return 0;
}
