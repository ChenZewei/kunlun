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

CSockStreamOB::~CSockStreamOB()
{
}

int CSockStreamOB::get_fd() const
{
	return getSocket();
}

void CSockStreamOB::work(CSockNotifier *psock_notifier, uint32_t nstatus)
{
	/*
	char inbuf[100];
	char outbuf[100];
	
	//缓冲区有数据可读
	if(nstatus & EPOLLIN){
		if(stream_recv(inbuf, sizeof(inbuf)) <= 0){
			printf("recv data failed\n");
			psock_notifier->detach(this);
			delete this;
			return ;
		}
		printf("recv data : %s\n", inbuf);
		psock_notifier->set_ob_status(this, EPOLLOUT | EPOLLET);
	}else if(nstatus & EPOLLOUT){
		strcpy(outbuf, "send test data from server");
		if(stream_send(outbuf, strlen(outbuf) + 1) <= 0){
			printf("send data failed\n");
			psock_notifier->detach(this);
			delete this;
			return ;
		}
		psock_notifier->set_ob_status(this, EPOLLIN | EPOLLET);
	}else{
		printf("undefined error\n");
	}
	*/
}
