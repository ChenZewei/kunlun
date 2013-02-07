#ifndef FILE_H_
#define FILE_H_
#include <sys/stat.h>
#include <fcntl.h>
/*
 * create a file object without buffer
 */
class CFile
{
public:
	/* 
	 * create a file object that isn't related with any files 
	 */
	CFile();
	CFile(const char *path, int flags);
	CFile(const char *path, int flags, mode_t mode);
	virtual ~CFile();

	virtual int Open(const char *path, int flags);
	virtual int Open(const char *path, int flags, mode_t mode);
	virtual int CreateFile(const char *path, mode_t mode);
	virtual int Unlink(const char *path);
	virtual int Lseek(off_t offset, int whence);
	virtual int Write(const void *buf, size_t count);
	virtual int Read(void *buf, size_t count);
	virtual void Close();

	int GetFileInfo(struct stat *buf);
	int GetErrorCode();
protected:
	int m_fd;
	int m_errno;
};
#endif //FILE_H_