#include <new>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include "rwlock.h"
#include "connector.h"
#include "msg_queue.h"
#include "vnode_info.h"
#include "common_types.h"
#include "timed_stream.h"
#include "storage_global.h"
#include "proxy_protocol.h"
#include "thread_beat_heart.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CThreadBeatHeart::CThreadBeatHeart(CInetAddr proxy_addr, \
	CMsgQueue *psync_msg_queue) : m_bstop_flag(false), \
	m_proxy_addr(proxy_addr), m_psync_msg_queue(psync_msg_queue)
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
					if(preport_stream != NULL) \
					{ \
						delete preport_stream; \
						preport_stream = NULL; \
					} \
					if(preport_resp_body != NULL) \
					{ \
						delete [] preport_resp_body; \
						preport_resp_body = NULL; \
					}

	int ret;
	int i, vnode_count;
	int vnode_id;
	int64_t body_len;
	byte vnode_status;
	time_t nlast_beat_time;
	char proxy_ip[KL_COMMON_IP_ADDR_LEN];
	CConnector *pconnector = NULL;
	CTimedStream *preport_stream = NULL;
	pkg_header report_resp_header;
	pkg_message *pstorage_sync_msg;
	byte *preport_resp_body = NULL;
	vnode_resp_unit *pvnode_resp_unit;
	storage_sync_event *pstorage_sync_event;

	try
	{
		pconnector = new CConnector(m_proxy_addr);
	}
	catch(std::bad_alloc)
	{
		pconnector = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create beat-hearting connector", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CConnector constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}
	
	try
	{
		preport_stream = new CTimedStream();
	}
	catch(std::bad_alloc)
	{
		preport_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create beat-hearting sock stream", \
			__LINE__);
		do_err();
		return ENOMEM;
	}
	catch(int errcode)
	{
		preport_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CTimedStream constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		return errcode;
	}

	if(pconnector->stream_connect(preport_stream) != 0)
	{
		m_proxy_addr.getipaddress(proxy_ip, sizeof(proxy_ip));
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"connect to proxy node(ip: %s, port: %d) failed, err: %s", \
			__LINE__, proxy_ip, m_proxy_addr.getport(), strerror(errno));
		do_err();
		return errno;
	}

	if(join_and_report(preport_stream) != 0)
	{
		m_proxy_addr.getipaddress(proxy_ip, sizeof(proxy_ip));
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send join request to proxy node(ip: %s, port: %d) failed", \
			__LINE__, proxy_ip, m_proxy_addr.getport());
		do_err();
		return -1;
	}

	nlast_beat_time = time(NULL);
	while(m_bstop_flag != true)
	{
		if(time(NULL) - nlast_beat_time < g_storage_beat_interval)
		{
			sleep(1);
			continue;
		}

		nlast_beat_time = time(NULL);
		if((ret = report_vnode_info(preport_stream)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"report storage vnode information failed, err: %s", \
				__LINE__, strerror(ret));
			do_err();
			return ret;
		}
		//to receive the reponse from proxy node
		if((ret = preport_stream->stream_recv(&report_resp_header, \
			sizeof(report_resp_header), g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"report thread recv report response header failed, err: %s", \
				__LINE__, strerror(ret));
			do_err();
			return ret;
		}

		//KL_SYS_INFOLOG("recv header ret: %d, pkg_header len: %d", ret, sizeof(pkg_header));
		//check the response status
		if(report_resp_header.status != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"storage node beat-hearting msg's response is occured with error", \
				__LINE__);
			do_err();
			return -1;
		}
		//check whether response message is legal
		body_len = CSERIALIZER::buff2int64(report_resp_header.pkg_len);
		if(body_len == 0)
		{
			//proxy resp msg, have no body to merge
			continue;
		}
		else if(report_resp_header.cmd == KL_PROXY_CMD_SLAVER_RESP)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"report thread recv slaver resp msg is illegal", \
				__LINE__);
			return -1;
		}
		if((body_len % sizeof(vnode_resp_unit) != 0))
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"report thread recv master resp body len(body_len: %d) is illegal", \
				__LINE__, body_len);
			do_err();
			return -1;
		}

		try
		{
			preport_resp_body = new byte[body_len];
		}
		catch(std::bad_alloc)
		{
			preport_resp_body = NULL;
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create report response body buffer(pkg_len: %ld)", \
				__LINE__, body_len);
			do_err();
			return ENOMEM;
		}
		//to receive response body
		if((ret = preport_stream->stream_recv(preport_resp_body, body_len, g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"report thread recv report response body failed, err: %s", \
				__LINE__, strerror(ret));
			do_err();
			return ret;
		}
		//KL_SYS_INFOLOG("recv body ret: %d, body len: %d", ret, body_len);
		//parse response body
		vnode_count = body_len / sizeof(vnode_resp_unit);
		for(i = 0; i < vnode_count; i++)
		{
			pvnode_resp_unit = (vnode_resp_unit *)(preport_resp_body + \
				i * sizeof(vnode_resp_unit));
			vnode_id = CSERIALIZER::buff2int32(pvnode_resp_unit->vnode_id);
			vnode_status = pvnode_resp_unit->vnode_status;

			//KL_SYS_INFOLOG("vnode_id: %d, vnode_status: %d", vnode_id, vnode_status);
			if((ret = merge_vnode_status(vnode_id, vnode_status)) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"report thread merge vnode status failed, vnode_id: %d, vnode_status: %d, " \
					"errcode: %d", \
					__LINE__, vnode_id, vnode_status, ret);
				do_err();
				return ret;
			}
			if(vnode_status != KL_REPLICA_STATUS_COPY_SRC && \
				vnode_status != KL_REPLICA_STATUS_MOVE_SRC)
				continue;

			try
			{
				pstorage_sync_event = new storage_sync_event();
			}
			catch(std::bad_alloc)
			{
				pstorage_sync_event = NULL;
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"no more memory to create storage sync event obj", \
					__LINE__);
				do_err();
				return ENOMEM;
			}
			try
			{
				pstorage_sync_msg = new pkg_message();
			}
			catch(std::bad_alloc)
			{
				pstorage_sync_msg = NULL;
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"no more memory to create storage sync msg obj", \
					__LINE__);
				delete pstorage_sync_event;
				do_err();
				return ENOMEM;
			}

			pstorage_sync_event->vnode_id = vnode_id;
			pstorage_sync_event->sync_dest_port = \
				CSERIALIZER::buff2int32(pvnode_resp_unit->sync_dest_port);
			memcpy(pstorage_sync_event->sync_dest_ip, pvnode_resp_unit->sync_dest_ip, \
				KL_COMMON_IP_ADDR_LEN);
			pstorage_sync_msg->pkg_len = sizeof(storage_sync_event);
			pstorage_sync_msg->pkg_ptr = (byte*)pstorage_sync_event;
			m_psync_msg_queue->push_msg(pstorage_sync_msg);
		}
		delete [] preport_resp_body;
		preport_resp_body = NULL;
	}
	do_err();
	return 0;
}

int CThreadBeatHeart::stop()
{
	m_bstop_flag = true;
	return 0;
}

int CThreadBeatHeart::join_and_report(CTimedStream *pjoin_stream)
{
	int ret;
	int64_t npkg_len;
	pkg_header join_msg_header;
	pkg_header proxy_resp_header;
	device_join_body join_msg_body;
	char proxy_ip[KL_COMMON_IP_ADDR_LEN];

	CSERIALIZER::long2buff(sizeof(join_msg_body), join_msg_header.pkg_len);
	join_msg_header.cmd = KL_PROXY_CMD_DEVICE_JOIN;
	join_msg_header.status = 0;
	if((ret = pjoin_stream->stream_send(&join_msg_header, sizeof(join_msg_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"join stream to send joining msg header failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}

	CSERIALIZER::int2buff(g_nstorage_zone_id, join_msg_body.zone_id);
	CSERIALIZER::int2buff(g_nstorage_weight, join_msg_body.weight);
	CSERIALIZER::int2buff(g_nstorage_bind_port, join_msg_body.device_bind_port);
	memcpy(join_msg_body.device_bind_ip, g_storage_bind_ip, KL_COMMON_IP_ADDR_LEN);

	if((ret = pjoin_stream->stream_send(&join_msg_body, sizeof(join_msg_body), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"join stream to send joining msg body failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}

	//wait to receive responsing data from master
	if((ret = pjoin_stream->stream_recv(&proxy_resp_header, sizeof(proxy_resp_header), \
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"join stream to recv joining response msg header failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}

	//check whether responsing msg header is correct
	npkg_len = CSERIALIZER::buff2int64(proxy_resp_header.pkg_len);
	if(npkg_len != 0 || proxy_resp_header.cmd != KL_PROXY_CMD_MASTER_RESP)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the resposing msg header of device joining is illegal", \
			__LINE__);
		return -1;
	}

	if(proxy_resp_header.status != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the current storage node joining in kunlun system failed", \
			__LINE__);
		return -1;
	}
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
	CSERIALIZER::int2buff(g_nstorage_bind_port, storagenode_info.device_bind_port);
	memcpy(storagenode_info.device_bind_ip, g_storage_bind_ip, KL_COMMON_IP_ADDR_LEN);
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
		g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send storage report header failed, err: %s", \
			__LINE__, strerror(ret));
		goto err_handle;
	}
	if((ret = preport_stream->stream_send(preport_body, body_len, g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send storage report body failed, err: %s", \
			__LINE__, strerror(ret));
		goto err_handle;
	}
	ret = 0;

err_handle:
	delete preport_body;
	preport_body = NULL;
	return ret;
}

int CThreadBeatHeart::merge_vnode_status(int vnode_id, byte vnode_status)
{
	int ret;
	int res;
	storage_vnode *pstorage_vnode_info;

	if((ret = g_pcontainer_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"wrlock container wrlock failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}

	//vnode's replica offline,so delete vnode info
	if(vnode_status == KL_REPLICA_STATUS_OFFLINE)
	{
		//always return 0
		g_pstorage_vnode_container->delete_vnode(vnode_id);
		ret = 0;
		goto do_return;
	}

	pstorage_vnode_info = g_pstorage_vnode_container->getvnode(vnode_id);
	if(pstorage_vnode_info != NULL)
	{
		pstorage_vnode_info->vnode_status = vnode_status;
		ret = 0;
		goto do_return;
	}
	//not found, so add vnode info to container
	try
	{
		pstorage_vnode_info = new storage_vnode();
	}
	catch(std::bad_alloc)
	{
		pstorage_vnode_info = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create storage vnode info obj", \
			__LINE__);
		ret = ENOMEM;
		goto do_return;
	}
	pstorage_vnode_info->vnode_id = vnode_id;
	pstorage_vnode_info->vnode_status = vnode_status;
	pstorage_vnode_info->vnode_version = 0;
	g_pstorage_vnode_container->add_vnode(pstorage_vnode_info);
	ret = 0;

do_return:
	if((res = g_pcontainer_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock container wrlock failed, err: %s", \
			__LINE__, strerror(res));
		ret = (ret == 0 ? res : ret);
	}
	return ret;
}