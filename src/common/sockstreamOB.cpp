#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include "inetaddr.h"
#include "sockstreamOB.h"

CSockStreamOB::CSockStreamOB() : CSockStream()
{
}

CSockStreamOB::CSockStreamOB(int sock) : \
		CSockStream(sock)
{
}

int CSockStreamOB::GetFd() const
{
	return getSocket();
}

void CSockStreamOB::Work(CSubject *pSubject, uint32_t nstatus)
{
	char inbuf[100];
	char outbuf[100];
	
	//缓冲区有数据可读
	if(nstatus & EPOLLIN){
		if(Recv(inbuf, sizeof(inbuf)) <= 0){
			printf("recv data failed\n");
			pSubject->Detach(this);
			delete this;
			return ;
		}
		printf("recv data : %s\n", inbuf);
		pSubject->SetOBStatus(this, EPOLLOUT | EPOLLET);
	}else if(nstatus & EPOLLOUT){
		strcpy(outbuf, "send test data from server");
		if(Send(outbuf, strlen(outbuf) + 1) <= 0){
			printf("send data failed\n");
			pSubject->Detach(this);
			delete this;
			return ;
		}
		pSubject->SetOBStatus(this, EPOLLIN | EPOLLET);
	}else{
		printf("undefined error\n");
	}
}
