#include <new>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "log.h"
#include "thread.h"
#include "rwlock.h"
#include "inetaddr.h"
#include "vnode_info.h"
#include "storage_global.h"
#include "connector.h"
#include "timed_stream.h"
#include "storage_server.h"
#include "common_types.h"
#include "proxy_protocol.h"
#include "thread_beat_heart.h"
#include "storage_server_conf.h"

CStorageServer::CStorageServer(CStorageServerConf storage_server_conf, \
	CStorageMsgParser *pstorage_msg_parser) : CBaseServer(storage_server_conf, \
	pstorage_msg_parser), m_storage_server_conf(storage_server_conf), \
	m_ppreport_threads(NULL)
{

}

CStorageServer::~CStorageServer()
{
	if(m_ppreport_threads != NULL)
	{
		delete [] m_ppreport_threads;
		m_ppreport_threads = NULL;
	}
}

int CStorageServer::run()
{
	if(join_and_report() != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"storage node join kunlun system and report to proxy node failed", \
			__LINE__);
		return -1;
	}
	return CBaseServer::run();
}

int CStorageServer::stop()
{
	int nthread_curr;
	for(nthread_curr = 0; nthread_curr < m_storage_server_conf.proxy_addr_list.size(); \
		nthread_curr++)
	{
		m_ppreport_threads[nthread_curr]->stop();
	}
	return CBaseServer::stop();
}

int CStorageServer::initilize()
{
	//todo: storage initilize
	g_timeout = m_storage_server_conf.ntimeout;
	strcpy(g_storage_bind_host, m_storage_server_conf.bind_host);
	g_storage_bind_port = m_storage_server_conf.nbind_port;
	
	try
	{
		g_pcontainer_rwlock = new CRWLock();
	}
	catch(std::bad_alloc)
	{
		g_pcontainer_rwlock = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CRWLock constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(g_pcontainer_rwlock == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode container rwlock", \
			__LINE__);
		return ENOMEM;
	}

	try
	{
		g_pstorage_vnode_container = new CStorageVnodeContainer();
	}
	catch(std::bad_alloc)
	{
		g_pstorage_vnode_container = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CStorageVnodeContainer constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(g_pstorage_vnode_container == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create storage vnode container", \
			__LINE__);
		return ENOMEM;
	}
	return CBaseServer::initilize();
}

int CStorageServer::join_and_report()
{
	typedef CThread* CThreadPtr;
	addr_list proxy_addr_list;

	proxy_addr_list = m_storage_server_conf.proxy_addr_list;
	if(proxy_addr_list.empty())
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no proxy server address to join and report", \
			__LINE__);
		return -1;
	}

	//we think the first address is the default master address
	//send joining message to master
	int proxy_port;
	int64_t npkg_len;
	int nthread_curr;
	CTimedStream join_stream;
	pkg_header join_msg_header;
	pkg_header master_resp_header;
	device_join_body join_msg_body;
	CThreadBeatHeart *pbeat_heart_func;
	char proxy_ip[KL_COMMON_IP_ADDR_LEN];
	CConnector master_connector(proxy_addr_list[0]);

	if(master_connector.stream_connect(&join_stream) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"connect to master failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}

	CSERIALIZER::long2buff(sizeof(join_msg_body), join_msg_header.pkg_len);
	join_msg_header.cmd = KL_PROXY_CMD_DEVICE_JOIN;
	join_msg_header.status = 0;
	if(join_stream.stream_send(&join_msg_header, sizeof(join_msg_header), \
		g_timeout) <= 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"join stream to send joining msg header failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}

	CInetAddr bind_addr(m_storage_server_conf.bind_host, \
		m_storage_server_conf.nbind_port);
	CSERIALIZER::int2buff(m_storage_server_conf.zone_id, join_msg_body.zone_id);
	CSERIALIZER::int2buff(m_storage_server_conf.weight, join_msg_body.weight);
	CSERIALIZER::int2buff(m_storage_server_conf.nbind_port, \
		join_msg_body.device_bind_port);
	bind_addr.getipaddress((char*)(join_msg_body.device_bind_ip), \
		sizeof(join_msg_body.device_bind_ip));
	
	if(join_stream.stream_send(&join_msg_body, sizeof(join_msg_body), \
		g_timeout) <= 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"join stream to send joining msg body failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}

	//wait to receive responsing data from master
	if(join_stream.stream_recv(&master_resp_header, sizeof(master_resp_header), \
		g_timeout) <= 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"join stream to recv joining responsing msg body failed, err: %s", \
			__LINE__, strerror(errno));
		return -1;
	}

	//check whether responsing msg header is correct
	npkg_len = CSERIALIZER::buff2int64(master_resp_header.pkg_len);
	if(npkg_len != 0 || master_resp_header.cmd != KL_PROXY_CMD_MASTER_RESP)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the resposing msg header of device joining is illegal", \
			__LINE__);
		return -1;
	}
	
	if(master_resp_header.status != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the current storage node joining in kunlun system failed", \
			__LINE__);
		return -1;
	}
	//join successfully
	//start beat-hearting thread to report
	try
	{
		m_ppreport_threads = new CThreadPtr[proxy_addr_list.size()];
	}
	catch(std::bad_alloc)
	{
		m_ppreport_threads = NULL;
	}
	if(m_ppreport_threads == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create report thread array", \
			__LINE__);
		return ENOMEM;
	}
	//create a report thread to each proxynode
	for(nthread_curr = 0; nthread_curr < proxy_addr_list.size(); nthread_curr++)
	{
		try
		{
			pbeat_heart_func = new CThreadBeatHeart(proxy_addr_list[nthread_curr]);
		}
		catch(std::bad_alloc)
		{
			pbeat_heart_func = NULL;
		}
		catch(int errcode)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call CThreadBeatHeart constructor failed, err: %s", \
				__LINE__, strerror(errcode));
			return errcode;
		}
		if(pbeat_heart_func == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create beat-hearting function", \
				__LINE__);
			return ENOMEM;
		}

		try
		{
			m_ppreport_threads[nthread_curr] = new CThread(pbeat_heart_func, \
				m_storage_server_conf.nthread_stack_size);
		}
		catch(std::bad_alloc)
		{
			m_ppreport_threads[nthread_curr] = NULL;
		}
		catch(int errcode)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call CThread constructor failed, err: %s", \
				__LINE__, strerror(errcode));
			return errcode;
		}
		if(m_ppreport_threads[nthread_curr] == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create beat-hearting thread", \
				__LINE__);
			return ENOMEM;
		}
	}

	//start master report thread
	if(m_ppreport_threads[0]->start() != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"start master report thread failed", \
			__LINE__);
		return -1;
	}
	//start slaver report thread
	for(nthread_curr = 1; nthread_curr < proxy_addr_list.size(); nthread_curr++)
	{
		if(m_ppreport_threads[nthread_curr]->start() != 0)
		{
			proxy_addr_list[nthread_curr].getipaddress(proxy_ip, sizeof(proxy_ip));
			proxy_port = proxy_addr_list[nthread_curr].getport();
			KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
				"start slaver(ip: %s, port: %d) report thread(thread id: %d) failed", \
				__LINE__, proxy_ip, proxy_port, nthread_curr);
			delete m_ppreport_threads[nthread_curr];
			m_ppreport_threads[nthread_curr] = NULL;
		}
	}
	return 0;
}