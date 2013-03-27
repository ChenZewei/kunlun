/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月14日 星期二 10时30分00秒
* File Name: acceptorOB.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include "sockstreamOB.h"
#include "acceptorOB.h"

CAcceptorOB::CAcceptorOB(int port, int backlog) : \
		CAcceptor(port, backlog)
{
}

CAcceptorOB::CAcceptorOB(CInetAddr& sockAddr, int backlog) \
		: CAcceptor(sockAddr, backlog)
{
}

int CAcceptorOB::GetFd() const
{
	return getSocket();
}

void CAcceptorOB::Work(CSubject *pSubject, uint32_t nstatus)
{
	int res;
	CSockStreamOB *pSockStreamOB;
	if(nstatus & EPOLLIN){
		while(1){
			pSockStreamOB = new CSockStreamOB();
			res = Accept(pSockStreamOB);
			if(res == -1){	//接收失败，或者没有连接可以接收
				delete pSockStreamOB;
				break;
			}
			pSubject->Attach(pSockStreamOB, EPOLLIN | EPOLLET);
		}
	}
}
