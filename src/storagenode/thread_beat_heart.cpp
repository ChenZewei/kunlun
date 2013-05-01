#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include "rwlock.h"
#include "connector.h"
#include "vnode_info.h"
#include "common_types.h"
#include "timed_stream.h"
#include "storage_global.h"
#include "proxy_protocol.h"
#include "thread_beat_heart.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CThreadBeatHeart::CThreadBeatHeart(CInetAddr proxy_addr) : \
	m_stop_flag(false), m_proxy_addr(proxy_addr)
{
}

CThreadBeatHeart::~CThreadBeatHeart()
{
}

int CThreadBeatHeart::run()
{
#define do_err()	if(pconnector != NULL) \
					{ \
						delete pconnector; \
						pconnector = NULL; \
					} \
					if(ptimed_stream != NULL) \
					{ \
						delete ptimed_stream; \
						ptimed_stream = NULL; \
					}
	int ret;
	char proxy_ip[KL_COMMON_IP_ADDR_LEN];
	CConnector *pconnector;
	CTimedStream *ptimed_stream;

	pconnector = kl_new CConnector(m_proxy_addr);
	if(pconnector == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create beat-hearting connector", \
			__LINE__);
		return ENOMEM;
	}
	
	ptimed_stream = kl_new CTimedStream();
	if(ptimed_stream == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create beat-hearting sock stream", \
			__LINE__);
		do_err();
		return ENOMEM;
	}

	if(pconnector->stream_connect(ptimed_stream) != 0)
	{
		m_proxy_addr.getipaddress(proxy_ip, sizeof(proxy_ip));
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"connect to proxy node(ip: %s, port: %d) failed, err: %s", \
			__LINE__, proxy_ip, m_proxy_addr.getport(), strerror(errno));
		do_err();
		return -1;
	}
	if(report_vnode_info(ptimed_stream) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"report storage vnode information failed", \
			__LINE__);
		do_err();
		return -1;
	}
	//to receive the reponse from proxy node
}

int CThreadBeatHeart::stop()
{
	return 0;
}

int CThreadBeatHeart::report_vnode_info(CTimedStream *preport_stream)
{
#ifdef _DEBUG
	assert(preport_stream);
#endif //_DEBUG
	int ret;
	int vnode_curr;
	int vnode_count;
	int64_t body_len;
	pkg_header report_header;
	storage_info storagenode_info;
	vnode_list_unit vnode_unit;
	byte *preport_body;
	storage_vnode *pstorage_vnode;

	if((ret = g_pcontainer_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"rdlock vnode container wrlock failed, err: %s", \
			__LINE__, strerror(errno));
		return ret;
	}
	vnode_count = g_pstorage_vnode_container->get_vnode_count();
	body_len = (int64_t)(vnode_count * sizeof(vnode_list_unit) + sizeof(storage_info));
	//serialize report header
	CSERIALIZER::long2buff(body_len, report_header.pkg_len);
	report_header.cmd = KL_PROXY_CMD_BEAT_HEART;
	report_header.status = 0;

	preport_body = kl_new byte[body_len];
	if(preport_body == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create report body", \
			__LINE__);
		g_pcontainer_rwlock->unlock();
		return ENOMEM;
	}

	//report storage info
	CSERIALIZER::int2buff(g_storage_bind_port, storagenode_info.device_bind_port);
	memcpy(storagenode_info.device_bind_ip, g_storage_bind_host, KL_COMMON_IP_ADDR_LEN);
	memcpy(preport_body, &storagenode_info, sizeof(storage_info));
	//report vnode info
	for(vnode_curr = 0; vnode_curr < vnode_count; vnode_curr++)
	{
		pstorage_vnode = g_pstorage_vnode_container->at(vnode_curr);
		//in theory, pstorage_vnode always is not null
#ifdef _DEBUG
		assert(pstorage_vnode);
#endif //_DEBUG
		vnode_unit.vnode_status = pstorage_vnode->vnode_status;
		CSERIALIZER::int2buff(pstorage_vnode->vnode_id, vnode_unit.vnode_id);
		CSERIALIZER::long2buff(pstorage_vnode->vnode_version, vnode_unit.vnode_version);
		memcpy(preport_body + vnode_curr * sizeof(vnode_unit) + sizeof(storage_info), \
			&vnode_unit, sizeof(vnode_list_unit));
	}

	if((ret = g_pcontainer_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock vnode container wrlock failed, err: %s", \
			__LINE__, strerror(errno));
		goto err_handle;
	}
	if((ret = preport_stream->stream_send(&report_header, sizeof(pkg_header), \
		g_timeout)) <= 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send storage report header failed, err: %s", \
			__LINE__, strerror(errno));
		goto err_handle;
	}
	if((ret = preport_stream->stream_send(preport_body, body_len, g_timeout)) <= 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send storage report body failed, err: %s", \
			__LINE__, strerror(errno));
		goto err_handle;
	}
	ret = 0;

err_handle:
	delete preport_body;
	preport_body = NULL;
	return ret;
}