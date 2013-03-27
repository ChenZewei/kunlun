#ifndef KL_COMMON_LOG_H_
#define KL_COMMON_LOG_H_

#define LOG_BUF_SIZE 1024

class CFile;
class CRWLock;

/* define log level */
enum LOG_LEVEL{LOG_LEVEL_ERROR, LOG_LEVEL_WARNNING, LOG_LEVEL_NOTICE, LOG_LEVEL_INFO};

class CLog
{
public:
	CLog(const char *path, LOG_LEVEL level);
	~CLog();

	int WriteLog(LOG_LEVEL level, const char *format, ...);
	/* WriteLog2() has two \n rather than WriteLog() has one */
	int WriteLog2(LOG_LEVEL level, const char *format, ...);
private:
	CLog(const CLog& log);
	CLog& operator= (const CLog& log);

	int doWriteLog(const char *buf);
	/* convert level from number to string */
	const char* strlevel(LOG_LEVEL level);
	/* format: 0000-00-00 00:00:00 */
	int GetLogTime(char *buf, int size);
private:
	CFile *m_pLogFile;
	CRWLock *m_plog_rwlock;
	LOG_LEVEL m_level;
};
#endif //KL_COMMON_LOG_H_