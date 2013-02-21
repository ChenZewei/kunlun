#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include "mutex.h"
#include "file.h"
#include "log.h"
#include "common_types.h"

CLog::CLog(const char *path, LOG_LEVEL level)
{
	m_pLogFile = new CFile(path, \
		O_RDWR | O_CREAT | O_APPEND, \
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(m_pLogFile == NULL){
		perror("open(or create) log file failed");
	}

	m_level = level;

	m_pLogMutex = new CMutex();
	if(m_pLogMutex == NULL){
		perror("create log lock failed");
	}
}

CLog::~CLog()
{
	if(m_pLogFile != NULL){
		delete m_pLogFile;
	}

	if(m_pLogMutex != NULL){
		delete m_pLogMutex;
	}
}

int CLog::WriteLog(LOG_LEVEL level, const char *format, ...)
{
	va_list ap;
	char msgbuf[LOG_BUF_SIZE];
	char buf[LOG_BUF_SIZE];
	int res;

	if(level > m_level){
		return 0; //ignore log
	}

	bzero(msgbuf, LOG_BUF_SIZE);
	va_start(ap, format);
	vsnprintf(msgbuf, LOG_BUF_SIZE, format, ap);
	va_end(ap);
	snprintf(buf, LOG_BUF_SIZE, "[%s]%s\n", \
		strlevel(level), msgbuf);

	m_pLogMutex->Lock();
	res = doWriteLog(buf);
	m_pLogMutex->Unlock();

	return res;
}

int CLog::WriteLog2(LOG_LEVEL level, const char *format, ...)
{
	va_list ap;
	char msgbuf[LOG_BUF_SIZE];
	char buf[LOG_BUF_SIZE];
	int res;

	if(level > m_level){
		return 0; //ignore log
	}

	bzero(msgbuf, LOG_BUF_SIZE);
	va_start(ap, format);
	vsnprintf(msgbuf, LOG_BUF_SIZE, format, ap);
	va_end(ap);
	snprintf(buf, LOG_BUF_SIZE, "[%s]%s\n\n", \
		strlevel(level), msgbuf);

	m_pLogMutex->Lock();
	res = doWriteLog(buf);
	m_pLogMutex->Unlock();

	return res;
}

int CLog::doWriteLog(const char *buf)
{
	char tbuf[KL_BUF_SIZE];
	if(GetLogTime(tbuf, KL_BUF_SIZE) == -1){
		printf("get local time failed\n");
		return -1;
	}
	
	//write
	if(m_pLogFile->Write(tbuf, strlen(tbuf)) \
		== -1){
		perror("write time to log failed");
		return -1;
	}
	m_pLogFile->Write(": ", 2);
	if(m_pLogFile->Write(buf, strlen(buf)) \
		== -1){
		perror("Write logmsg to log failed");
		return -1;
	}
	return 0;
}

int CLog::GetLogTime(char *buf, int size)
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
	switch(level){
	case LOG_LEVEL_ERROR:
		return "ERROR";
	case LOG_LEVEL_WARNNING:
		return "WARNNING";
	case LOG_LEVEL_NOTICE:
		return "NOTICE";
	case LOG_LEVEL_INFO:
		return "INFO";
	}
	return NULL;
}