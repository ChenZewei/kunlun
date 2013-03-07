#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "file.h"
#include "common_types.h"

CFile::CFile()
{
	m_fd = -1;
	m_errno = 0;
}

CFile::CFile(const char *path, int flags)
{
	char msgbuf[KL_BUF_SIZE];
	m_fd = -1;
	m_errno = 0;
	if(Open(path, flags) == -1){
		bzero(msgbuf, KL_BUF_SIZE);
		snprintf(msgbuf, KL_BUF_SIZE, \
			"open file: %s failed", \
			path);
		perror(msgbuf);
	}
}

CFile::CFile(const char *path, int flags, mode_t mode)
{
	char msgbuf[KL_BUF_SIZE];
	m_fd = -1;
	m_errno = 0;
	if(Open(path, flags, mode) == -1){
		bzero(msgbuf, KL_BUF_SIZE);
		snprintf(msgbuf, KL_BUF_SIZE, \
			"open file: %s failed", \
			path);
		perror(msgbuf);
	}
}

CFile::~CFile()
{
	Close();
}

void CFile::Close()
{
	if(m_fd != -1){
		close(m_fd);
	}
}

int CFile::Open(const char *path, int flags)
{
	m_fd = open(path, flags);
	if(m_fd == -1){
		m_errno = errno;
		return -1;
	}
	return 0;
}

int CFile::Open(const char *path, int flags, mode_t mode)
{
	m_fd = open(path, flags, mode);
	if(m_fd == -1){
		m_errno = errno;
		return -1;
	}
	return 0;
}

int CFile::CreateFile(const char *path, mode_t mode)
{
	m_fd = creat(path, mode);
	if(m_fd == -1){
		m_errno = errno;
		return -1;
	}
	return 0;
}

int CFile::GetErrorCode()
{
	return m_errno;
}

int CFile::GetFileInfo(struct stat *buf)
{
	if(fstat(m_fd, buf) == -1){
		m_errno = errno;
		return -1;
	}
	return 0;
}

int CFile::Read(void *buf, size_t count)
{
	char *p;
	p = static_cast<char*>(buf);
	size_t nread = 0;
	size_t nleft = count;

	while(nleft > 0){
		nread = read(m_fd, p, nleft);
		if(nread  == -1){
			if(errno == EINTR){
				continue; // read was interrupted, and call read() again
			}
			return -1;
		}else if(nread == 0){
			break; // EOF
		}
		p += nread;
		nleft -= nread;
	}

	return count - nleft;
}

int CFile::Write(const void *buf, size_t count)
{
	const char *p;
	p = static_cast<const char*>(buf);
	size_t nwrite = 0;
	size_t nleft = count;

	while(nleft > 0){
		nwrite = write(m_fd, p, nleft);
		if(nwrite == -1){
			if(errno == EINTR){
				continue;
			}
			return -1;
		}else if(nwrite == 0){
			return -1;
		}

		p += nwrite;
		nleft -= nwrite;
	}

	return count - nleft;
}

int CFile::Lseek(off_t offset, int whence)
{
	return lseek(m_fd, offset, whence);
}

int CFile::Unlink(const char *path)
{
	return unlink(path);
}