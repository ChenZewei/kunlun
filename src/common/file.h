#ifndef KL_COMMON_FILE_H_
#define KL_COMMON_FILE_H_
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
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
	bool file_exist(const char *file_name);
	int64_t get_file_size();
	int get_file_info(struct stat *buf);
protected:
	int m_fd;
};
#endif //KL_COMMON_FILE_H_