#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "log.h"
#include "proxy_global.h"
#include "proxy_server.h"
#include "proxy_msg_parser.h"
#include "proxy_server_conf.h"

void sigusrhandler(int sig);
void sighuphandler(int sig);
void sigquithandler(int sig);
void sigpipehandler(int sig);

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("error args, Usage %s <port>\n", argv[0]);
		return -1;
	}

	int ret;
	int nbind_port;
	struct sigaction act;
	CProxyServer *pproxy_server;
	CProxyMsgParser *pproxy_msg_parser;
	CProxyServerConf proxy_server_conf;

	nbind_port = atoi(argv[1]);
	if(nbind_port < 0){
		return -1;
	}

	proxy_server_conf.bind_host = NULL;
	proxy_server_conf.nbind_port = nbind_port;
	proxy_server_conf.nlog_level = 4; //DEBUG level
	proxy_server_conf.nthread_stack_size = 1 * 1024 * 1024;
	proxy_server_conf.ntimeout = 5;
	proxy_server_conf.nwork_thread_count = 10;
	proxy_server_conf.sys_log_path = "./kunlun_proxy.log";
	proxy_server_conf.bmaster_flag = true;
	proxy_server_conf.nnamespace_power = 32;
	proxy_server_conf.nreplica_count = 3;
	proxy_server_conf.nvnode_count = 1000;

	try
	{
		pproxy_msg_parser = new CProxyMsgParser();
	}
	catch(std::bad_alloc)
	{
		pproxy_msg_parser = NULL;
	}
	try
	{
		pproxy_server = new CProxyServer(proxy_server_conf, pproxy_msg_parser);
	}
	catch(std::bad_alloc)
	{
		pproxy_server = NULL;
	}
	catch(int errcode)
	{
		printf("file: "__FILE__", line: %d, " \
			"call CProxyServer contructor failed, err: %s\n", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(pproxy_server == NULL)
	{
		printf("file: "__FILE__", line: %d, " \
			"no more memory to create kunlun proxy server\n", \
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

	act.sa_handler = sigpipehandler;
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

	if((ret = pproxy_server->initilize()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"initilize kunlun proxy server failed, errcode: %d", \
			__LINE__, ret);
		return ret;
	}
	KL_SYS_NOTICELOG("kunlun(proxyd) proxynode server has been started, " \
		"and listen on port: %d. All copyrights are reserved " \
		"by Leslie Wei.  Connect with me by e-mail: leslieyuchen@gmail.com", \
		proxy_server_conf.nbind_port);
	pproxy_server->run();
	KL_SYS_NOTICELOG("kunlun(proxyd) proxynode server exit...");
	delete pproxy_server;
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

void sigpipehandler(int sig)
{
	//printf("catch a SIGPIPE signal, ignore\n");
}