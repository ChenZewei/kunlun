#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "log.h"
#include "inetaddr.h"
#include "storage_global.h"
#include "storage_server.h"
#include "storage_msg_parser.h"
#include "storage_server_conf.h"

void sigusrhandler(int sig);
void sighuphandler(int sig);
void sigquithandler(int sig);

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("error args, Usage %s <conf_path>\n", argv[0]);
		return -1;
	}

	int ret;
	struct sigaction act;
	CStorageServer *pstorage_server;
	CStorageMsgParser *pstorage_msg_parser;
	CStorageServerConf storage_server_conf;
	
	//should get info from storage conf file
	CInetAddr proxy_addr("localhost", 6000);
	storage_server_conf.bind_host = "localhost";
	storage_server_conf.nbind_port = 6001;
	storage_server_conf.nlog_level = 4; //DEBUG level
	storage_server_conf.nthread_stack_size = 1 * 1024 * 1024;
	storage_server_conf.ntimeout = 5;
	storage_server_conf.nwork_thread_count = 10;
	storage_server_conf.sys_log_path = "./kunlun_storage.log";
	storage_server_conf.zone_id = 0;
	storage_server_conf.weight = 1;
	storage_server_conf.proxy_addr_list.push_back(proxy_addr);

	try
	{
		pstorage_msg_parser = new CStorageMsgParser();
	}
	catch(std::bad_alloc)
	{
		pstorage_msg_parser = NULL;
	}
	try
	{
		pstorage_server = new CStorageServer(storage_server_conf, pstorage_msg_parser);
	}
	catch(std::bad_alloc)
	{
		pstorage_server = NULL;
	}
	catch(int errcode)
	{
		printf("file: "__FILE__", line: %d, " \
			"call CStorageServer contructor failed, err: %s\n", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(pstorage_server == NULL)
	{
		printf("file: "__FILE__", line: %d, " \
			"no more memory to create kunlun storage server\n", \
			__LINE__);
		return ENOMEM;
	}

	memset(&act, 0, sizeof(act));
	sigemptyset(&act.sa_mask);

	act.sa_handler = sigusrhandler;
	if(sigaction(SIGUSR1, &act, NULL) < 0 || \
		sigaction(SIGUSR2, &act, NULL) < 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call sigaction fail, err: %s", \
			__LINE__, strerror(errno));
		return errno;
	}

	act.sa_handler = sighuphandler;
	if(sigaction(SIGHUP, &act, NULL) < 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call sigaction fail, err: %s", \
			__LINE__, strerror(errno));
		return errno;
	}

	act.sa_handler = SIG_IGN;
	if(sigaction(SIGPIPE, &act, NULL) < 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call sigaction fail, err: %s", \
			__LINE__, strerror(errno));
		return errno;
	}

	/*act.sa_handler = sigquithandler;
	if(sigaction(SIGINT, &act, NULL) < 0 || \
		sigaction(SIGTERM, &act, NULL) < 0 || \
		sigaction(SIGABRT, &act, NULL) < 0 || \
		sigaction(SIGQUIT, &act, NULL) < 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call sigaction fail, err: %s", \
			__LINE__, strerror(errno));
		return errno;
	}*/

	if((ret = pstorage_server->initilize()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"initilize kunlun proxy server failed, errcode: %d", \
			__LINE__, ret);
		return ret;
	}
	KL_SYS_NOTICELOG("kunlun(storaged) storagenode server has been started, " \
		"and listen on port: %d. All copyrights are reserved " \
		"by Leslie Wei.  Connect with me by e-mail: leslieyuchen@gmail.com", \
		storage_server_conf.nbind_port);
	pstorage_server->run();
	KL_SYS_NOTICELOG("kunlun(storaged) storagenode server exit...");
	delete pstorage_server;
	destroy_global_data();
	return 0;
}

void sigusrhandler(int sig)
{

}

void sighuphandler(int sig)
{
	printf("catch a SIGHUP signal, ignore\n");
}

void sigquithandler(int sig)
{

}