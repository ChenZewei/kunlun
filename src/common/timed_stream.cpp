#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include "log.h"
#include "timed_stream.h"

CTimedStream::CTimedStream() : CSockStream()
{
}

CTimedStream::CTimedStream(int sock) : CSockStream(sock)
{
}

CTimedStream::CTimedStream(int sock, bool bclosed) : \
	CSockStream(sock, bclosed)
{
}

CTimedStream::~CTimedStream()
{
}

int CTimedStream::stream_send(const void *buf, size_t len, int timeout)
{
	//use poll
	int ret;
	size_t nleft_bytes;
	ssize_t nsend_bytes;
	struct pollfd poll_event;
	const unsigned char *ptr;

	poll_event.fd = getsocket();
	poll_event.events = POLLOUT;
	poll_event.revents = 0;

	/*if((poll(&poll_event, 1, 1000 * timeout) == -1) || \
		((poll_event.revents & POLLOUT) == 0))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"poll wait writing event failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}
	return CSockStream::stream_send(buf, len);*/
	nleft_bytes = len;
	ptr = (const unsigned char *)buf;
	while(nleft_bytes > 0)
	{
		nsend_bytes = CSockStream::stream_send(ptr, nleft_bytes);
		if(nsend_bytes >= 0)
		{
			nleft_bytes -= nsend_bytes;
			ptr += nsend_bytes;
			continue;
		}
		else
		{
			if(!(errno == EAGAIN || errno == EWOULDBLOCK))
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"timed stream send data failed, err: %s", \
					__LINE__, strerror(errno));
				return errno != 0 ? errno : EINTR;
			}
		}

		if(((ret = poll(&poll_event, 1, 1000 * timeout)) == -1) || \
			((poll_event.revents & POLLOUT) == 0))
		{
			errno = errno != 0 ? errno : EINTR;
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"poll wait writing event failed, err: %s", \
				__LINE__, strerror(errno));
			return errno;
		}
		if(ret == 0)
		{
			errno = ETIMEDOUT;
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"poll wait writing event failed, err: %s", \
				__LINE__, strerror(errno));
			return errno;
		}
	}
	return 0;
}

int CTimedStream::stream_recv(void *buf, size_t len, int timeout)
{
	//use poll
	int ret;
	size_t nleft_bytes;
	ssize_t nrecv_bytes;
	unsigned char *ptr;
	struct pollfd poll_event;

	poll_event.fd = getsocket();
	poll_event.events = POLLIN;
	poll_event.revents = 0;

	/*if((poll(&poll_event, 1, 1000 * timeout) == -1) || \
		((poll_event.revents & POLLIN) == 0))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"poll wait reading event failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}
	return CSockStream::stream_recv(buf, len);*/
	nleft_bytes = len;
	ptr = (unsigned char *)buf;
	while(nleft_bytes > 0)
	{
		nrecv_bytes = CSockStream::stream_recv(ptr, nleft_bytes);
		if(nrecv_bytes > 0)
		{
			nleft_bytes -= nrecv_bytes;
			ptr += nrecv_bytes;
			continue;
		}

		if(nrecv_bytes < 0)
		{
			if(!(errno == EAGAIN || errno == EWOULDBLOCK))
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"timed stream recv data failed, err: %s", \
					__LINE__, strerror(errno));
				return errno != 0 ? errno : EINTR;
			}
		}
		else //sockstream peer closed
		{
			return ENOTCONN;
		}

		if(((ret = poll(&poll_event, 1, 1000 * timeout)) == -1) || \
			((poll_event.revents & POLLIN) == 0))
		{
			errno = errno != 0 ? errno : EINTR;
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"poll wait reading event failed, err: %s", \
				__LINE__, strerror(errno));
			return errno;
		}
		if(ret == 0)
		{
			errno = ETIMEDOUT;
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"poll wait reading event failed, err: %s", \
				__LINE__, strerror(errno));
			return errno;
		}
	}
	return 0;
}