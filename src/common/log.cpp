#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "rwlock.h"
#include "file.h"
#include "log.h"

CLog *g_psys_log = NULL;

CLog::CLog(const char *path, LOG_LEVEL level)
{
	m_plog_file = new CFile(path, O_RDWR | O_CREAT | O_APPEND, \
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(m_plog_file == NULL){
		printf("file: "__FILE__", line: %d, "\
			"open log file failed, err: %s\n", \
			__LINE__, strerror(ENOMEM));
		return;
	}

	m_level = level;
	m_plog_rwlock = new CRWLock();
	if(m_plog_rwlock == NULL){
		printf("file: "__FILE__", line: %d, "\
			"create log lock failed, err: %s\n", \
			__LINE__, strerror(ENOMEM));
		return;
	}
#ifdef _DEBUG
	printf("call CLog constructor successfully\n");
#endif //_DEBUG
}

CLog::~CLog()
{
	if(m_plog_file != NULL){
		delete m_plog_file;
		m_plog_file = NULL;
	}

	if(m_plog_rwlock != NULL){
		delete m_plog_rwlock;
		m_plog_rwlock = NULL;
	}
#ifdef _DEBUG
	printf("call CLog destructor successfully\n");
#endif
}

int CLog::writelog(LOG_LEVEL level, const char *format, ...)
{
	va_list ap;
	char msgbuf[KL_COMMON_LOG_BUF_SIZE];
	char buf[KL_COMMON_LOG_BUF_SIZE];
	int res;

	if(level > m_level){
		return 0; //ignore log
	}

	bzero(msgbuf, KL_COMMON_LOG_BUF_SIZE);
	va_start(ap, format);
	vsnprintf(msgbuf, KL_COMMON_LOG_BUF_SIZE, format, ap);
	va_end(ap);
	snprintf(buf, KL_COMMON_LOG_BUF_SIZE, "[%s]%s\n", \
		strlevel(level), msgbuf);

	if(m_plog_rwlock->wrlock() == -1){
		printf("file: "__FILE__", line: %d, " \
			"call wrlock failed, err: %s\n", \
			__LINE__, strerror(errno));
		return -1;
	}
	res = dowritelog(buf);
	if(m_plog_rwlock->unlock() == -1){
		printf("file: "__FILE__", line: %d, " \
			"call unlock failed, err: %s\n", \
			__LINE__, strerror(errno));
		return -1;
	}

	return res;
}

int CLog::dowritelog(const char *buf)
{
	char tbuf[32];

	if(getlogtime(tbuf, sizeof(tbuf)) == -1){
		printf("file: "__FILE__", line: %d, "\
			"get local time failed, err: %s\n", \
			__LINE__, strerror(errno));
		return -1;
	}
	
	//write
	if(m_plog_file->write_file(tbuf, strlen(tbuf)) \
		== -1){
		//perror("write time to log failed");
		//kl_perror("write time to log failed, " \
		//	"err: %s\n", strerror(errno));
		printf("file: "__FILE__", line: %d, "\
			"write time to log failed, err: %s\n", \
			__LINE__, strerror(errno));
		return -1;
	}
	m_plog_file->write_file(": ", 2);
	if(m_plog_file->write_file(buf, strlen(buf)) \
		== -1){
		//perror("Write logmsg to log failed");
		//kl_perror("Write logmsg to log failed, " \
		//		"err: %s\n", strerror(errno));
		printf("file: "__FILE__", line: %d, "\
			"write logmsg to log failed, err: %s\n", \
			__LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

int CLog::getlogtime(char *buf, int size)
{
	struct tm *ptm;
	time_t tn;
	bzero(buf, size);

	tn = time(NULL);
	ptm = localtime(&tn);
	if(ptm == NULL){
		return -1;
	}

	snprintf(buf, size, \
		"%.4d-%.2d-%.2d %.2d:%.2d:%.2d", \
		ptm->tm_year + 1900, \
		ptm->tm_mon + 1, \
		ptm->tm_mday + 1, \
		ptm->tm_hour, \
		ptm->tm_min, \
		ptm->tm_sec);
	return 0;
}

const char* CLog::strlevel(LOG_LEVEL level)
{
	switch(level)
	{
	case LOG_LEVEL_ERROR:
		return "ERROR";
	case LOG_LEVEL_WARNNING:
		return "WARNNING";
	case LOG_LEVEL_NOTICE:
		return "NOTICE";
	case LOG_LEVEL_INFO:
		return "INFO";
	case LOG_LEVEL_DEBUG:
		return "DEBUG";
	}
	return NULL;
}