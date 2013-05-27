#include <new>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include "file_reader_stream.h"

CFileReaderStream::CFileReaderStream(const char *path, CSockStream *psock_stream) : \
	CFile(path, O_RDONLY), m_psock_stream(psock_stream)
{

}

CFileReaderStream::~CFileReaderStream()
{

}

int CFileReaderStream::stream_readout()
{
	int ret;
	off_t nfile_size;
	off_t noff_set;
	off_t nleft_bytes;
	ssize_t nsend_bytes;
	
	struct stat stat_buf;
	if((ret = get_file_info(&stat_buf)) != 0)
	{
		return ret;
	}
	nfile_size = stat_buf.st_size;
	lseek_file(0, SEEK_SET);

	ret = 0;
	noff_set = 0;
	nleft_bytes = nfile_size;
	m_psock_stream->setblocking();
	while(nleft_bytes > 0)
	{
		nsend_bytes = sendfile(m_psock_stream->getsocket(), m_fd, &noff_set, nleft_bytes);
		if(nsend_bytes <= 0)
		{
			ret = (errno != 0 ? errno : EIO);
			break;
		}
		nleft_bytes -= nsend_bytes;
	}
	m_psock_stream->setnonblocking();
	return ret;
}