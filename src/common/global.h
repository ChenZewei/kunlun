#ifndef KL_COMMON_GLOBAL_H_
#define KL_COMMON_GLOBAL_H_

class CLog;
extern CLog *g_psys_log;
#define KL_SYS_ERRLOG(format, ...)	\
	g_psys_log->WriteLog(LOG_LEVEL_ERROR, format, \
	__VA_ARGS__)
#define KL_SYS_WARNNINGLOG(format, ...)	\
	g_psys_log->WriteLog(LOG_LEVEL_WARNNING, format, \
	__VA_ARGS__)
#define KL_SYS_NOTICELOG(format, ...)	\
	g_psys_log->WriteLog(LOG_LEVEL_NOTICE, format, \
	__VA_ARGS__)
#define KL_SYS_INFOLOG(format, ...)	\
	g_psys_log->WriteLog(LOG_LEVEL_INFO, format, \
	__VA_ARGS__)

#endif //KL_COMMON_GLOBAL_H_