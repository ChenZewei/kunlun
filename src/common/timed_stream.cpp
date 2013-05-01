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
	struct pollfd poll_event;

	poll_event.fd = getsocket();
	poll_event.events = POLLOUT;
	poll_event.revents = 0;

	if((poll(&poll_event, 1, 1000 * timeout) == -1) || \
		((poll_event.revents & POLLOUT) == 0))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"poll wait writing event failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}
	return CSockStream::stream_send(buf, len);
}

int CTimedStream::stream_recv(void *buf, size_t len, int timeout)
{
	//use poll
	struct pollfd poll_event;

	poll_event.fd = getsocket();
	poll_event.events = POLLIN;
	poll_event.revents = 0;

	if((poll(&poll_event, 1, 1000 * timeout) == -1) || \
		((poll_event.revents & POLLIN) == 0))
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"poll wait reading event failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}
	return CSockStream::stream_recv(buf, len);
}