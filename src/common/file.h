#ifndef KL_COMMON_FILE_H_
#define KL_COMMON_FILE_H_
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
	virtual int create_file(const char *path, mode_t mode);
	virtual int Unlink(const char *path);
	virtual int Lseek(off_t offset, int whence);
	virtual int Write(const void *buf, size_t count);
	virtual int Read(void *buf, size_t count);
	virtual void Close();

	int get_file_info(struct stat *buf);
	int get_error_code();
protected:
	int m_fd;
	int m_errno;
};
#endif //KL_COMMON_FILE_H_