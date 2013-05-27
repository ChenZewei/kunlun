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
	int nbind_port;
	//server timeout option
	int ntimeout;
	//server bind ip address or host name
	char bind_host[256];
	//system log path
	char sys_log_path[256];
	//log level number
	int nlog_level;
	//thread stack size
	int nthread_stack_size;
};
#endif //KL_COMMON_BASE_SERVER_CONF_H_