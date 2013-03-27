/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月14日 星期二 00时02分19秒
* File Name: acceptor.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "acceptor.h"

CAcceptor::CAcceptor(int port, int backlog) : \
		CSock(AF_INET, SOCK_STREAM)
{
	CInetAddr sockAddr(NULL, port);
	if(bind(m_fd, sockAddr.getsockaddr(), sizeof(struct sockaddr)) \
			== -1){
		perror("bind error");
		_exit(-1);
	}
	listen(m_fd, backlog);
	setNonBlocking();
}

CAcceptor::CAcceptor(CInetAddr& sockAddr, int backlog) : \
		CSock(AF_INET, SOCK_STREAM)
{
	if(bind(m_fd, sockAddr.getsockaddr(), sizeof(struct sockaddr)) \
			== -1){
		perror("bind error");
		_exit(-1);
	}
	listen(m_fd, backlog);
	setNonBlocking();
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
	pSockStream->SetSockStream(connfd);
	return 0;
}
