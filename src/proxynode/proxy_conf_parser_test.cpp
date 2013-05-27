#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proxy_server_conf.h"
#include "proxy_conf_parser.h"

int main(int argc, char *argv[])
{
	CProxyServerConf proxy_conf;
	try
	{
		CProxyConfParser proxy_conf_parser(argv[1]);
		if(proxy_conf_parser.parse_conf(&proxy_conf) != 0)
		{
			fprintf(stderr, "file: "__FILE__", line: %d, " \
				"parse conf file failed\n", \
				__LINE__);
			return -1;
		}
	}
	catch(std::bad_alloc)
	{
		fprintf(stderr, "file: "__FILE__", line: %d, " \
			"no more memory to parse conf file\n", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		fprintf(stderr, "file: "__FILE__", line: %d, " \
			"parse conf file failed, err: %s\n", \
			__LINE__, strerror(errcode));
		return errcode;
	}

	printf("proxy_conf.bind_host = %s\n", proxy_conf.bind_host);
	printf("proxy_conf.nbind_port = %d\n", proxy_conf.nbind_port);
	printf("proxy_conf.nlog_level = %d\n", proxy_conf.nlog_level);
	printf("proxy_conf.nthread_stack_size = %d\n", proxy_conf.nthread_stack_size);
	printf("proxy_conf.ntimeout = %d\n", proxy_conf.ntimeout);
	printf("proxy_conf.nwork_thread_count = %d\n", proxy_conf.nwork_thread_count);
	printf("proxy_conf.sys_log_path = %s\n", proxy_conf.sys_log_path);
	printf("proxy_conf.bmaster_flag = %s\n", proxy_conf.bmaster_flag == true ? "true" : "false");
	printf("proxy_conf.nnamespace_power = %d\n", proxy_conf.nnamespace_power);
	printf("proxy_conf.nreplica_count = %d\n", proxy_conf.nreplica_count);
	printf("proxy_conf.nvnode_count = %d\n", proxy_conf.nvnode_count);
	return 0;
}