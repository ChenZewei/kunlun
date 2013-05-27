#include <new>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "log.h"
#include "mutex.h"
#include "base_server.h"
#include "msg_parser.h"
#include "common_types.h"
#include "common_protocol.h"
#include "timed_stream.h"
#include "base_server_conf.h"

#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

#define gettid() syscall(SYS_gettid)

#define KL_COMMON_PROTOCOL_MSG_TEST1 0
//#define KL_COMMON_PROTOCOL_MSG_TEST2 1
//#define KL_COMMON_PROTOCOL_MSG_TEST3 2
#define KL_COMMON_PROTOCOL_MSG_SERVER_RESP 3
typedef pkg_header base_msg_header;
CMutex g_console_mutex;

class CBaseMsgParser : public CMsgParser
{
public:
	int parse_msg(pkg_message* pkg_msg_ptr);
	int msg_test1_handle(pkg_message* pkg_msg_ptr);
	//int msg_test2_callback(pkg_message* pkg_msg_ptr);
	//int msg_test3_callback(pkg_message* pkg_msg_ptr);
};

int CBaseMsgParser::parse_msg(pkg_message* pkg_msg_ptr)
{
#ifdef _DEBUG
	assert(pkg_msg_ptr);
#endif //_DEBUG
	byte msg_cmd = *(pkg_msg_ptr->pkg_ptr);
	switch(msg_cmd)
	{
	case KL_COMMON_PROTOCOL_MSG_TEST1:
		return msg_test1_handle(pkg_msg_ptr);
	default:
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"catch undefined msg...", \
			__LINE__);
		return -1;
	}
}

int CBaseMsgParser::msg_test1_handle(pkg_message* pkg_msg_ptr)
{
	int res;
	char *pbody;
	char resp_body[KL_COMMON_BUF_SIZE];
	CTimedStream *presp_stream;
	base_msg_header msg_resp_header;

	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		presp_stream = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CTimedStream constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(presp_stream == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create sock stream obj", \
			__LINE__);
		res = -1;
		goto error_handle;
	}

	try
	{
		pbody = new char[pkg_msg_ptr->pkg_len - 1];
	}
	catch(std::bad_alloc)
	{
		pbody = NULL;
	}
	if(pbody == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create msg body array", \
			__LINE__);
		res = -1;
		goto error_handle;
	}
	memcpy(pbody, pkg_msg_ptr->pkg_ptr + 2, pkg_msg_ptr->pkg_len - 2);
	*(pbody + pkg_msg_ptr->pkg_len - 2) = '\0';
	g_console_mutex.lock();
	printf("msg_test1_callback(thread: %ld) data : %s, and server(thread: %ld) response\n", \
		gettid(), pbody, gettid());
	delete [] pbody;
	g_console_mutex.unlock();

	msg_resp_header.cmd = KL_COMMON_PROTOCOL_MSG_SERVER_RESP;
	msg_resp_header.status = 0;
	snprintf(resp_body, KL_COMMON_BUF_SIZE, \
		"msg_test1_callback(thread: %ld) response the request", \
		gettid());
	CSERIALIZER::long2buff(strlen(resp_body), msg_resp_header.pkg_len);
	if((res = presp_stream->stream_send(&msg_resp_header, \
		sizeof(base_msg_header), 5)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send responsed msg header failed, err: %s", \
			__LINE__, strerror(res));
		goto error_handle;
	}

	if((res = presp_stream->stream_send(resp_body, strlen(resp_body), 5)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send responsed msg body failed, err: %s", \
			__LINE__, strerror(res));
		goto error_handle;
	}
	res = 0;
error_handle:
	//consume a msg, so delete the msg pkg
	delete pkg_msg_ptr;
	pkg_msg_ptr = NULL;

	if(presp_stream != NULL)
	{
		delete presp_stream;
		presp_stream = NULL;
	}
	return res;
}

void sighuphandler(int sig);
void sigpipehandler(int sig);

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("error args, Usage %s <port>\n", argv[0]);
		return -1;
	}

	int nbind_port;
	int ret;
	struct sigaction act;
	CBaseServer *pbase_server;
	CMsgParser *pmsg_parser;
	CBaseServerConf base_server_conf;

	nbind_port = atoi(argv[1]);
	if(nbind_port < 0){
		return -1;
	}

	base_server_conf.bind_host = NULL;
	base_server_conf.nbind_port = nbind_port;
	base_server_conf.nlog_level = 4; //DEBUG level
	base_server_conf.nthread_stack_size = 1 * 1024 * 1024;
	base_server_conf.ntimeout = 5;
	base_server_conf.nwork_thread_count = 10;
	base_server_conf.sys_log_path = "./kunlun_sys.log";
	
	try
	{
		pmsg_parser = new CBaseMsgParser();
	}
	catch(std::bad_alloc)
	{
		pmsg_parser = NULL;
	}
	try
	{
		pbase_server = new CBaseServer(base_server_conf, pmsg_parser);
	}
	catch(std::bad_alloc)
	{
		pbase_server = NULL;
	}
	catch(int errcode)
	{
		printf("file: "__FILE__", line: %d, " \
			"call CBaseServer constructor failed, err: %s\n", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(pbase_server == NULL)
	{
		printf("file: "__FILE__", line: %d, " \
			"no more memory to create base server", \
			__LINE__);
		return ENOMEM;
	}

	memset(&act, 0, sizeof(act));
	sigemptyset(&act.sa_mask);
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

	if((ret = pbase_server->initilize()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"initilize base server failed, errcode: %d", \
			__LINE__, ret);
		return ret;
	}
	KL_SYS_NOTICELOG("kunlun(common test) base server has been started, " \
					 "and listen on port: %d. All copyrights are reserved " \
					 "by Leslie Wei.  Connect with me by e-mail: leslieyuchen@gmail.com", \
					 base_server_conf.nbind_port);
	pbase_server->run();
	KL_SYS_NOTICELOG("epollserver exit...");
	delete pbase_server;
	delete g_psys_log;
	return 0;
}

void sighuphandler(int sig)
{
	printf("catch a SIGHUP signal, ignore\n");
}

void sigpipehandler(int sig)
{
	printf("catch a SIGPIPE signal, ignore\n");
}