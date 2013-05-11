#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "sockstream.h"

CSockStream::CSockStream() : CSock(), m_bclosed(true)
{
}

CSockStream::CSockStream(int sock) : m_bclosed(true)
{
	setsockstream(sock);
}

CSockStream::CSockStream(int sock, bool bclosed) : \
	m_bclosed(bclosed)
{
	setsockstream(sock);
}

CSockStream::~CSockStream()
{
	/*
	 * set m_fd to -1, just to avoid closing the socket when we call
	   CSock destructor
	 */
	if(!m_bclosed)
		m_fd = -1;
}

void CSockStream::setsockstream(int sock)
{
	m_fd = sock;
	setnonblocking();
}

int CSockStream::stream_send(const void *buf, size_t len)
{
	/*int nleft;
	int nwrite;
	const unsigned char *ptr;

	nleft = len;
	nwrite = 0;
	ptr = (const unsigned char*)buf;

	while(nleft > 0)
	{
		nwrite = send(m_fd, ptr, nleft, 0);
		if(nwrite < 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				usleep(1000);
				continue;
			} //was interrupted or buffer is full
			return -1; //other error
		}
		else if(nwrite == 0)
		{
			return 0;	//opposite side was closed
		}

		nleft -= nwrite;
		ptr += nwrite;
	}
	return len - nleft;*/
	return send(m_fd, buf, len, 0);
}

int CSockStream::stream_recv(void *buf, size_t len)
{
	/*int nleft;
	int nread;
	unsigned char *ptr;

	nleft = len;
	nread = 0;
	ptr = (unsigned char*)buf;

	while(nleft > 0)
	{
		nread = recv(m_fd, ptr, nleft, 0);
		if(nread < 0)
		{
			if(errno == EINTR)
			{
				//was interrupted
				usleep(1000);
				continue;
			}
			else if(errno == EAGAIN)
			{
				//buffer is empty
				return len - nleft;
			}
			return -1;
		}
		else if(nread == 0)
		{
			//opposite side was closed
			return 0;
		}

		nleft -= nread;
		ptr += nread;
	}

	return len - nleft;*/
	return recv(m_fd, buf, len, 0);
}