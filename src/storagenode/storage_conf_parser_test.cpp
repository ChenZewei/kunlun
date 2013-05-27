#include <new>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_server_conf.h"
#include "storage_conf_parser.h"

int main(int argc, char *argv[])
{
	int i;
	char ip_addr[16];
	CStorageServerConf storage_conf;
	
	try
	{
		CStorageConfParser storage_conf_parser(argv[1]);
		if(storage_conf_parser.parse_conf(&storage_conf) != 0)
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

	printf("storage_conf.bind_host = %s\n", storage_conf.bind_host);
	printf("storage_conf.nbind_port = %d\n", storage_conf.nbind_port);
	printf("storage_conf.nlog_level = %d\n", storage_conf.nlog_level);
	printf("storage_conf.nthread_stack_size = %d\n", storage_conf.nthread_stack_size);
	printf("storage_conf.ntimeout = %d\n", storage_conf.ntimeout);
	printf("storage_conf.nwork_thread_count = %d\n", storage_conf.nwork_thread_count);
	printf("storage_conf.sys_log_path = %s\n", storage_conf.sys_log_path);
	printf("storage_conf.device_path = %s\n", storage_conf.device_path);
	printf("storage_conf.nweight = %d\n", storage_conf.nweight);
	printf("storage_conf.nzone_id = %d\n", storage_conf.nzone_id);
	printf("storage_conf.proxy_addr_list:\n");
	for(i = 0; i < storage_conf.proxy_addr_list.size(); i++)
	{
		storage_conf.proxy_addr_list[i].getipaddress(ip_addr, 16);
		printf("ip: %s, port: %d\n", \
			ip_addr, \
			storage_conf.proxy_addr_list[i].getport());
	}
	return 0;
}