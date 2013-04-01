#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "log.h"
#include "thread.h"
#include "msg_queue.h"
#include "acceptorOB.h"
#include "msg_looper.h"
#include "msg_parser.h"
#include "base_server.h"
#include "thread_msg_recv.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CBaseServer::CBaseServer(CBaseServerConf base_server_conf, CMsgParser *pmsg_parser) : \
	m_base_server_conf(base_server_conf), m_pmsg_parser(pmsg_parser)
{
#ifdef _DEBUG
	assert(m_pmsg_parser);
#endif //_DEBUG
	initilize();
}

CBaseServer::~CBaseServer()
{
	if(m_pmsg_queue_arr != NULL)
	{
		delete m_pmsg_queue_arr;
		m_pmsg_queue_arr = NULL;
	}

	//every thread obj will be deleted by themselves, 
	//so needn't to delete thread obj, just delete thread obj's container
	if(m_ppthread != NULL)
	{
		delete m_ppthread;
		m_ppthread = NULL;
	}
	if(m_pmsg_parser != NULL)
	{
		delete m_pmsg_parser;
		m_pmsg_parser = NULL;
	}
}

int CBaseServer::initilize()
{
	typedef CThread* CThreadPtr;

	int nwork_thread_curr;
	LOG_LEVEL sys_log_level;
	CAcceptorOB *pacceptor_ob;
	CThreadMsgRecv *pthread_msg_recv;
	CMsgLooper *pmsg_looper;

	m_work_thread_count = m_base_server_conf.nwork_thread_count;

	//initilize system log
	sys_log_level = (LOG_LEVEL)(m_base_server_conf.nlog_level);
	g_psys_log = new CLog(m_base_server_conf.sys_log_path, \
		sys_log_level);
	if(g_psys_log == NULL)
	{
		printf("file: "__FILE__", line: %d, " \
			"no more memory for object g_psys_log", \
			__LINE__);
		return ENOMEM;
	}

	//create a message queue for every work thread
	m_pmsg_queue_arr = new CMsgQueueArr(m_work_thread_count);
	if(m_pmsg_queue_arr == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"no more memory for server to create msg queue array", \
			__LINE__);
		return ENOMEM;
	}

	m_ppthread = new CThreadPtr[m_work_thread_count + 1];
	if(m_ppthread == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"no more memory for server to create thread array", \
			__LINE__);
		return ENOMEM;
	}

	pacceptor_ob = new CAcceptorOB(m_base_server_conf.bind_host, \
		m_base_server_conf.nbind_prot, 1024, m_base_server_conf.ntimeout, \
		m_pmsg_queue_arr);
	if(pacceptor_ob == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"no more memory for server to create acceptor object", \
			__LINE__);
		return ENOMEM;
	}

	//bug: maybe memory leak
	//initilize msg recv thread
	pthread_msg_recv = new CThreadMsgRecv(pacceptor_ob);
	if(pthread_msg_recv == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"no more memory for server to create msg recv thread func object", \
			__LINE__);
		return ENOMEM;
	}
	m_ppthread[0] = new CThread(pthread_msg_recv, \
		m_base_server_conf.nthread_stack_size, true);
	if(m_ppthread[0] == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"no more memory for server to create thread object", \
			__LINE__);
		return ENOMEM;
	}

	//initilize msg work thread
	for(nwork_thread_curr = 1; nwork_thread_curr <= m_work_thread_count; \
		nwork_thread_curr++)
	{
		pmsg_looper = new CMsgLooper(m_pmsg_queue_arr->getmsgqueuebyrobin(), m_pmsg_parser);
		if(pmsg_looper == NULL)
		{
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"no more memory for server to create msg looper", \
				__LINE__);
			return ENOMEM;
		}
		m_ppthread[nwork_thread_curr] = new CThread(pmsg_looper, \
			m_base_server_conf.nthread_stack_size, true);
		if(m_ppthread[nwork_thread_curr] == NULL)
		{
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"no more memory for server to create msg work thread", \
				__LINE__);
			return ENOMEM;
		}
	}
	return 0;
}

int CBaseServer::run()
{
	int nthread_curr;
	int res;
	
	for(nthread_curr = 0; nthread_curr < m_work_thread_count + 1; \
		nthread_curr++)
	{
		if((res = m_ppthread[nthread_curr]->start()) != 0)
		{
			KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
				"start to run thread[%d] failed, err: %s", \
				__LINE__, nthread_curr, strerror(res));
			return -1;
		}
	}
	KL_SYS_NOTICELOG("server has been started");
	
	while(true)
	{
		/*
		 * wait for a signal to arrive, block forever
		 */
		select(0, NULL, NULL, NULL, NULL);
	}
	return 0;
}

int CBaseServer::stop()
{
	m_stop_flag = true;
	return 0;
}