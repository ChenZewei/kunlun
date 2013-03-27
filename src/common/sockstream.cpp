/*******************************************
* Author: Leslie Wei
* Created Time: 2012年08月13日 星期一 23时24分55秒
* File Name: sockstream.cpp
* Description: 
* @Copyright reserved
********************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "sockstream.h"

CSockStream::CSockStream() : CSock()
{
}

CSockStream::CSockStream(int sock)
{
	SetSockStream(sock);
}

void CSockStream::SetSockStream(int sock)
{
	m_fd = sock;
	setNonBlocking();
}

int CSockStream::Send(const void *buf, size_t len)
{
	int nleft;
	int nwrite;
	const unsigned char *ptr;

	nleft = len;
	nwrite = 0;
	ptr = (const unsigned char*)buf;

	while(nleft > 0){
		nwrite = send(m_fd, ptr, nleft, 0);
		if(nwrite < 0){
			if(errno == EINTR || errno == EAGAIN){
				usleep(1000);
				continue;
			} //被中断抢占或者写缓冲区满
			return -1; //其它错误
		}else if(nwrite == 0){
			return 0;	//对方被关闭
		}

		nleft -= nwrite;
		ptr += nwrite;
	}
	return len - nleft;
}

int CSockStream::Recv(void *buf, size_t len)
{
	int nleft;
	int nread;
	unsigned char *ptr;

	nleft = len;
	nread = 0;
	ptr = (unsigned char*)buf;

	while(nleft > 0){
		nread = recv(m_fd, ptr, nleft, 0);
		if(nread < 0){
			if(errno == EINTR){
				//被中断抢占
				usleep(1000);
				continue;
			}else if(errno == EAGAIN){
				//缓冲区无数据，读操作结束
				return len - nleft;
			}
			return -1;
		}else if(nread == 0){
			//对方已关闭
			return 0;
		}

		nleft -= nread;
		ptr += nread;
	}

	return len - nleft;
}
