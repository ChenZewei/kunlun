#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "file.h"

CFile::CFile()
{
	m_fd = -1;
	m_errno = 0;
}

CFile::CFile(const char *path, int flags)
{
	m_fd = -1;
	m_errno = 0;
	if(open_file(path, flags) == -1){
		printf("file: "__FILE__", line: %d, " \
			"open file(path: %s) failed, err: %s\n", \
			__LINE__, path, strerror(errno));
		return;
	}
#ifdef _DEBUG
	printf("call CFile constructor successfully\n");
#endif //_DEBUG
}

CFile::CFile(const char *path, int flags, mode_t mode)
{
	m_fd = -1;
	m_errno = 0;
	if(open_file(path, flags, mode) == -1){
		printf("file: "__FILE__", line: %d, " \
			"open file(path: %s) failed, err: %s\n", \
			__LINE__, path, strerror(errno));
		return;
	}
#ifdef _DEBUG
	printf("call CFile constructor successfully\n");
#endif //_DEBUG
}

CFile::~CFile()
{
	close_file();
}

void CFile::close_file()
{
	if(m_fd != -1){
		close(m_fd);
	}
}

int CFile::open_file(const char *path, int flags)
{
	m_fd = open(path, flags);
	if(m_fd == -1){
		m_errno = errno;
		return -1;
	}
	m_errno = 0;
	return 0;
}

int CFile::open_file(const char *path, int flags, mode_t mode)
{
	m_fd = open(path, flags, mode);
	if(m_fd == -1){
		m_errno = errno;
		return -1;
	}
	m_errno = 0;
	return 0;
}

int CFile::create_file(const char *path, mode_t mode)
{
	m_fd = creat(path, mode);
	if(m_fd == -1){
		m_errno = errno;
		return -1;
	}
	m_errno = 0;
	return 0;
}

int CFile::get_error_code()
{
	return m_errno;
}

int CFile::get_file_info(struct stat *buf)
{
	if(fstat(m_fd, buf) == -1){
		m_errno = errno;
		return -1;
	}
	m_errno = 0;
	return 0;
}

int CFile::read_file(void *buf, size_t count)
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
			m_errno = errno;
			return -1;
		}else if(nread == 0){
			break; // EOF
		}
		p += nread;
		nleft -= nread;
	}
	m_errno = errno;
	return count - nleft;
}

int CFile::write_file(const void *buf, size_t count)
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
			m_errno = errno;
			return -1;
		}else if(nwrite == 0){
			m_errno = errno;
			return -1;
		}

		p += nwrite;
		nleft -= nwrite;
	}

	m_errno = 0;
	return count - nleft;
}

int CFile::lseek_file(off_t offset, int whence)
{
	int res;
	res = lseek(m_fd, offset, whence);
	m_errno = errno;
	return res;
}

int CFile::unlink_file(const char *path)
{
	int res;
	res =  unlink(path);
	m_errno = errno;
	return res;
}