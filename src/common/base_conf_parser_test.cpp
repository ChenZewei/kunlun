#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base_server_conf.h"
#include "base_conf_parser.h"

int main(int argc, char *argv[])
{
	CBaseServerConf base_conf;
	try
	{
		CBaseConfParser base_conf_parser(argv[1]);
		if(base_conf_parser.parse_conf(&base_conf) != 0)
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

	printf("base_conf.bind_host = %s\n", base_conf.bind_host);
	printf("base_conf.nbind_port = %d\n", base_conf.nbind_port);
	printf("base_conf.nlog_level = %d\n", base_conf.nlog_level);
	printf("base_conf.nthread_stack_size = %d\n", base_conf.nthread_stack_size);
	printf("base_conf.ntimeout = %d\n", base_conf.ntimeout);
	printf("base_conf.nwork_thread_count = %d\n", base_conf.nwork_thread_count);
	printf("base_conf.sys_log_path = %s\n", base_conf.sys_log_path);
	return 0;
}