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
#include "connector.h"
#include "msg_queue.h"
#include "directory.h"
#include "vnode_info.h"
#include "timed_stream.h"
#include "common_types.h"
#include "storage_global.h"
#include "storage_server.h"
#include "proxy_protocol.h"
#include "thread_sync_data.h"
#include "thread_beat_heart.h"
#include "storage_server_conf.h"

CStorageServer::CStorageServer(CStorageServerConf storage_server_conf, \
	CStorageMsgParser *pstorage_msg_parser) : CBaseServer(storage_server_conf, \
	pstorage_msg_parser), m_storage_server_conf(storage_server_conf), \
	m_ppreport_threads(NULL), m_psync_msg_queue(NULL), m_psync_thread(NULL)
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
	int ret;
	int proxy_port;
	int nthread_curr;
	addr_list proxy_addr_list;
	char proxy_ip[KL_COMMON_IP_ADDR_LEN];

	//we think the first address is the default master address
	//start master report thread
	if(m_ppreport_threads[0]->start() != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"start master report thread failed", \
			__LINE__);
		return -1;
	}

	//start slaver report thread
	proxy_addr_list = m_storage_server_conf.proxy_addr_list;
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

	if((ret = m_psync_thread->start()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"start storage sync thread failed, err: %s", \
			__LINE__, strerror(ret));
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
	typedef CThread* CThreadPtr;
	CDirectory dir;
	int nthread_curr, ret;
	addr_list proxy_addr_list;
	CThreadBeatHeart *pbeat_heart_func;
	CThreadSyncData *psync_thread_func;
	
	g_ntimeout = m_storage_server_conf.ntimeout;
	g_nstorage_bind_port = m_storage_server_conf.nbind_port;
	g_nstorage_zone_id = m_storage_server_conf.nzone_id;
	g_nstorage_weight = m_storage_server_conf.nweight;
	memcpy(g_device_root, m_storage_server_conf.device_path, KL_COMMON_PATH_LEN);

	CInetAddr bind_addr(m_storage_server_conf.bind_host, \
		m_storage_server_conf.nbind_port);
	if(bind_addr.getipaddress(g_storage_bind_ip, KL_COMMON_IP_ADDR_LEN) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"get storage node bind ip failed, err: %s", \
			__LINE__, strerror(errno));
		return errno;
	}
	
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

	//create sync message queue
	try
	{
		m_psync_msg_queue = new CMsgQueue();
	}
	catch(std::bad_alloc)
	{
		m_psync_msg_queue = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create sync msg queue", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CMsgQueue constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	//create sync push thread
	proxy_addr_list = m_storage_server_conf.proxy_addr_list;
	if(proxy_addr_list.empty())
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no proxy server address to join and report", \
			__LINE__);
		return -1;
	}
	try
	{
		psync_thread_func = new CThreadSyncData(m_psync_msg_queue, proxy_addr_list[0]);
	}
	catch(std::bad_alloc)
	{
		psync_thread_func = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create sync thread function", \
			__LINE__);
		return ENOMEM;
	}
	try
	{
		m_psync_thread = new CThread(psync_thread_func, \
			m_storage_server_conf.nthread_stack_size);
	}
	catch(std::bad_alloc)
	{
		m_psync_thread = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create storage sync thread", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		m_psync_thread = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CThread constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}

	//create beat-hearting thread to report
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
			pbeat_heart_func = new CThreadBeatHeart(proxy_addr_list[nthread_curr], \
				m_psync_msg_queue, nthread_curr);
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
	//make device root dir
	if(dir.dir_exist(g_device_root))
	{
		dir.remove_dir(g_device_root);
	}
	if((ret = dir.make_dir(g_device_root)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"make device root dir failed, dir_path: %s, err: %s", \
			__LINE__, g_device_root, strerror(ret));
		return ret;
	}
	//do base server initilize
	return CBaseServer::initilize();
}