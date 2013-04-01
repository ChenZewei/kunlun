#ifndef KL_COMMON_BASE_SERVER_CONF_H_
#define KL_COMMON_BASE_SERVER_CONF_H_
class CBaseServerConf
{
public:
	CBaseServerConf() {}
	virtual ~CBaseServerConf() {}
	//the work thread size of server
	int nwork_thread_count;
	//server bind port
	int nbind_prot;
	//server timeout option
	int ntimeout;
	//server bind ip address or host name
	const char *bind_host;
	//system log path
	const char *sys_log_path;
	//log level number
	int nlog_level;
	//thread stack size
	int nthread_stack_size;
};
#endif //KL_COMMON_BASE_SERVER_CONF_H_