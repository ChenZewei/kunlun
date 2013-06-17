#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "hash.h"
#include "rwlock.h"
#include "proxy_global.h"
#include "proxy_server.h"
#include "node_container.h"
#include "proxy_msg_parser.h"

CProxyServer::CProxyServer(CProxyServerConf proxy_server_conf, \
	CProxyMsgParser *pproxy_msg_parser) : CBaseServer(proxy_server_conf, \
	pproxy_msg_parser), m_proxy_server_conf(proxy_server_conf)
{

}

CProxyServer::~CProxyServer()
{

}

int CProxyServer::initilize()
{
	//do proxy initilize
	g_bmaster_flag = m_proxy_server_conf.bmaster_flag;
	//g_namespace_power = m_proxy_server_conf.nnamespace_power;
	g_ntimeout = m_proxy_server_conf.ntimeout;
	
	try
	{
		g_pdevice_chg_rwlock = new CRWLock();
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create device chg rwlock", \
			__LINE__);
		g_pdevice_chg_rwlock = NULL;
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CRWLock contructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}

	try
	{
		g_pvnode_container = new CVnodeContainer(m_proxy_server_conf.nvnode_count, \
			m_proxy_server_conf.nreplica_count);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode container", \
			__LINE__);
		g_pvnode_container = NULL;
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CVnodeContainer constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	if(g_pvnode_container->initilize() != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call vnode container initilize failed", \
			__LINE__);
		return -1;
	}

	try
	{
		g_pdevice_container = new CDeviceContainer();
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create device container", \
			__LINE__);
		g_pdevice_container = NULL;
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CDeviceContainer constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}

	try
	{
		g_pnamespace_hash = new CNameSpaceHash(m_proxy_server_conf.nnamespace_power, \
			m_proxy_server_conf.nvnode_count);
	}
	catch(std::bad_alloc)
	{
		g_pnamespace_hash = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create namespace hash, err: %s", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		g_pnamespace_hash = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CNameSpaceHash constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	return CBaseServer::initilize();
}

int CProxyServer::run()
{
	return CBaseServer::run();
}

int CProxyServer::stop()
{
	return CBaseServer::stop();
}