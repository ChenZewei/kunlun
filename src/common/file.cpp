#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "file.h"

CFile::CFile() : m_fd(-1)
{
	
}

CFile::CFile(const char *path, int flags) : m_fd(-1)
{
	if(open_file(path, flags) == -1){
		printf("file: "__FILE__", line: %d, " \
			"open file(path: %s) failed, err: %s\n", \
			__LINE__, path, strerror(errno));
		throw errno;
	}
}

CFile::CFile(const char *path, int flags, mode_t mode) : m_fd(-1)
{
	if(open_file(path, flags, mode) == -1){
		printf("file: "__FILE__", line: %d, " \
			"open file(path: %s) failed, err: %s\n", \
			__LINE__, path, strerror(errno));
		throw errno;
	}
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
		return -1;
	}
	return 0;
}

int CFile::open_file(const char *path, int flags, mode_t mode)
{
	m_fd = open(path, flags, mode);
	if(m_fd == -1){
		return -1;
	}
	return 0;
}

int CFile::create_file(const char *path, mode_t mode)
{
	m_fd = creat(path, mode);
	if(m_fd == -1){
		return -1;
	}
	return 0;
}

int CFile::get_file_info(struct stat *buf)
{
	if(fstat(m_fd, buf) == -1){
		return errno;
	}
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
			return -1;
		}else if(nread == 0){
			break; // EOF
		}
		p += nread;
		nleft -= nread;
	}
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
			return -1;
		}else if(nwrite == 0){
			return -1;
		}

		p += nwrite;
		nleft -= nwrite;
	}

	return count - nleft;
}

int CFile::lseek_file(off_t offset, int whence)
{
	int res;
	res = lseek(m_fd, offset, whence);
	return res;
}

int CFile::unlink_file(const char *path)
{
	int res;
	res =  unlink(path);
	return res;
}

int64_t CFile::get_file_size()
{
	struct stat buf;
	if(get_file_info(&buf) != 0)
	{
		return -1;
	}
	return (int64_t)(buf.st_size);
}

bool CFile::file_exist(const char *file_name)
{
	if(access(file_name, F_OK) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}