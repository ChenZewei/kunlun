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

	virtual int open_file(const char *path, int flags);
	virtual int open_file(const char *path, int flags, mode_t mode);
	virtual int create_file(const char *path, mode_t mode);
	virtual int unlink_file(const char *path);
	virtual int lseek_file(off_t offset, int whence);
	virtual int write_file(const void *buf, size_t count);
	virtual int read_file(void *buf, size_t count);
	virtual void close_file();

	int get_file_info(struct stat *buf);
	int get_error_code();
protected:
	int m_fd;
	int m_errno;
};
#endif //KL_COMMON_FILE_H_