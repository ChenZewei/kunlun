#ifndef KL_COMMON_LOG_H_
#define KL_COMMON_LOG_H_

#define KL_COMMON_LOG_BUF_SIZE 1024

class CFile;
class CRWLock;

/* define log level */
enum LOG_LEVEL{LOG_LEVEL_ERROR, LOG_LEVEL_WARNNING, LOG_LEVEL_NOTICE, LOG_LEVEL_INFO};

class CLog
{
public:
	CLog(const char *path, LOG_LEVEL level);
	~CLog();

	int writelog(LOG_LEVEL level, const char *format, ...);
	/* WriteLog2() has two \n rather than WriteLog() has one */
	int writelog2(LOG_LEVEL level, const char *format, ...);
private:
	CLog(const CLog& log);
	CLog& operator= (const CLog& log);

	int doWriteLog(const char *buf);
	/* convert level from number to string */
	const char* strlevel(LOG_LEVEL level);
	/* format: 0000-00-00 00:00:00 */
	int getlogtime(char *buf, int size);
private:
	CFile *m_plog_file;
	CRWLock *m_plog_rwlock;
	LOG_LEVEL m_level;
};

extern CLog *g_psys_log;
#define KL_SYS_ERRLOG(format, ...)	\
	g_psys_log->writelog(LOG_LEVEL_ERROR, format, \
	##__VA_ARGS__)
#define KL_SYS_WARNNINGLOG(format, ...)	\
	g_psys_log->writelog(LOG_LEVEL_WARNNING, format, \
	##__VA_ARGS__)
#define KL_SYS_NOTICELOG(format, ...)	\
	g_psys_log->writelog(LOG_LEVEL_NOTICE, format, \
	##__VA_ARGS__)
#define KL_SYS_INFOLOG(format, ...)	\
	g_psys_log->writelog(LOG_LEVEL_INFO, format, \
	##__VA_ARGS__)
#endif //KL_COMMON_LOG_H_