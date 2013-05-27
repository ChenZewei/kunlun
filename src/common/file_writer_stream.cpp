#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include "common_types.h"
#include "file_writer_stream.h"

CFileWriterStream::CFileWriterStream(const char *path, CSockStream *psock_stream) : \
	CFile(path, O_WRONLY | O_CREAT, 0644), m_psock_stream(psock_stream)
{

}

CFileWriterStream::~CFileWriterStream()
{

}

int CFileWriterStream::stream_writein(int64_t nfile_size)
{
	/*char file_buf[KL_COMMON_BUF_SIZE];
	int nleft_bytes;
	int nrecv_bytes;
	
	nleft_bytes = nfile_size;
	while(nleft_bytes > 0)
	{
		nrecv_bytes = nleft_bytes > KL_COMMON_BUF_SIZE ? KL_COMMON_BUF_SIZE : \
			nleft_bytes;
		if(m_ptimed_stream->stream_recv(file_buf, nrecv_bytes, ntimeout) != 0)
		{
			return -1;
		}
		nleft_bytes -= nrecv_bytes;
		if(write_file(file_buf, nrecv_bytes) <= 0)
		{
			return -1;
		}
	}*/
	int ret;
	off_t noff_set;
	off_t nleft_bytes;
	ssize_t nwrite_bytes;

	ret = 0;
	noff_set = 0;
	nleft_bytes = nfile_size;
	m_psock_stream->setblocking();
	lseek_file(0, SEEK_SET);
	while(nleft_bytes > 0)
	{
		nwrite_bytes = sendfile(m_fd, m_psock_stream->getsocket(), &noff_set, nleft_bytes);
		if(nwrite_bytes <= 0)
		{
			ret = (errno != 0 ? errno : EIO);
			break;
		}
		nleft_bytes -= nwrite_bytes;
	}
	m_psock_stream->setnonblocking();
	return ret;
}