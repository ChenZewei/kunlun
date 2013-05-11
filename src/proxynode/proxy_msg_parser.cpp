#include <new>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "rwlock.h"
#include "inetaddr.h"
#include "timed_stream.h"
#include "proxy_global.h"
#include "node_container.h"
#include "vnode_balancer.h"
#include "proxy_protocol.h"
#include "proxy_msg_parser.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CProxyMsgParser::CProxyMsgParser()
{
}

CProxyMsgParser::~CProxyMsgParser()
{
}

int CProxyMsgParser::parse_msg(pkg_message* pkg_msg_ptr)
{
#ifdef _DEBUG
	assert(pkg_msg_ptr);
#endif //_DEBUG
	byte msg_cmd;
	if(pkg_msg_ptr->pkg_ptr == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the context of message package is null", \
			__LINE__);
		return -1;
	}

	msg_cmd = *(pkg_msg_ptr->pkg_ptr);
	KL_SYS_INFOLOG("proxy msg parser parse msg, msg cmd = %d", msg_cmd);
	switch(msg_cmd)
	{
	case KL_PROXY_CMD_DEVICE_JOIN:
		return msg_device_join_handle(pkg_msg_ptr);
	case KL_PROXY_CMD_BEAT_HEART:
		return msg_device_report_handle(pkg_msg_ptr);
	default:
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"proxy msg parser catch undefined msg...", \
			__LINE__);
		return -1;
	}
	return 0;
}

int CProxyMsgParser::msg_device_join_handle(pkg_message* pkg_msg_ptr)
{
	int res;
	int ret;
	int zone_id;
	int weight;
	int device_bind_port;
	char *pdevice_bind_ip;
	pdevice_join_body pbody;
	pkg_header resp_header;
	CTimedStream *presp_stream;
	device_info_ptr pdevice_info;
	CVnodeBalancer *pvnode_balancer;

	if(sizeof(device_join_body) != pkg_msg_ptr->pkg_len - 2)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the msg pkg(msg_cmd = %d) of device join is illegal", \
			__LINE__, pkg_msg_ptr->pkg_ptr);
		delete pkg_msg_ptr;
		return -1;
	}

	pbody = (pdevice_join_body)((pkg_msg_ptr->pkg_ptr) + 2);
	pdevice_bind_ip = (char*)(pbody->device_bind_ip);
	//if peer ip is "0.0.0.0", try to get the peer true ip
	if(strcmp(pdevice_bind_ip, "0.0.0.0") == 0)
	{
		CInetAddr peer_addr;
		if(CTimedStream(pkg_msg_ptr->sock_stream_fd, \
			false).getpeeraddr(&peer_addr) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get storage node address failed", \
				__LINE__);
			delete pkg_msg_ptr;
			return -1;
		}
		KL_SYS_INFOLOG("get peer address successfully");
		peer_addr.getipaddress(pdevice_bind_ip, KL_COMMON_IP_ADDR_LEN);
	}

	device_bind_port = CSERIALIZER::buff2int32(pbody->device_bind_port);
	zone_id = CSERIALIZER::buff2int32(pbody->zone_id);
	weight = CSERIALIZER::buff2int32(pbody->weight);

	try
	{
		pdevice_info = new device_info();
	}
	catch(std::bad_alloc)
	{
		pdevice_info = NULL;
	}
	if(pdevice_info == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create a device info obj, " \
			"device bind_ip: %s, bind_port: %d", \
			__LINE__, pdevice_bind_ip, device_bind_port);
		delete pkg_msg_ptr;
		return ENOMEM;
	}
	memcpy(pdevice_info->bind_ip, pdevice_bind_ip, KL_COMMON_IP_ADDR_LEN);
	pdevice_info->nbind_port = device_bind_port;
	pdevice_info->device_id = 0;
	pdevice_info->weight = weight;
	pdevice_info->zone_id = zone_id;

	try
	{
		pvnode_balancer = new CVnodeBalancer();
	}
	catch(std::bad_alloc)
	{
		pvnode_balancer = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode balancer", \
			__LINE__);
		delete pkg_msg_ptr;
		delete pdevice_info;
		return ENOMEM;
	}
	KL_SYS_INFOLOG("do vnode balance");
	if((res = g_pdevice_chg_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"wrlock the g_device_chg_rwlock failed, err: %s", \
			__LINE__, strerror(res));
		delete pkg_msg_ptr;
		delete pdevice_info;
		delete pvnode_balancer;
		return -1;
	}
	//slaver operation
	if(g_bmaster_flag == false)
	{
		ret = pvnode_balancer->slaver_do_device_join(pdevice_info);
	}
	else if(g_pdevice_container->get_node_count() < \
		g_pvnode_container->get_replica_count())
	{
		ret = pvnode_balancer->master_do_copy_vnode(pdevice_info);
	}
	else
	{
		ret = pvnode_balancer->master_do_move_vnode(pdevice_info);
	}
	KL_SYS_INFOLOG("do vnode balance ret = %d", ret);
	
	if(ret != 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"device joining failed, device info: ip: %s, port: %d", \
			__LINE__, pdevice_info->bind_ip, pdevice_info->nbind_port);
		g_pvnode_container->destroy_all_replicas_info(pdevice_info);
		delete pdevice_info;
		pdevice_info = NULL;
	}

#ifdef _DEBUG
	g_pdevice_container->putout_vnode_count();
#endif //_DEBUG

	if((res = g_pdevice_chg_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock the g_device_chg_rwlock failed, err: %s", \
			__LINE__, strerror(res));
		delete pkg_msg_ptr;
		delete pvnode_balancer;
		return -1;
	}

	CSERIALIZER::long2buff(0, resp_header.pkg_len);
	resp_header.cmd = g_bmaster_flag ? KL_PROXY_CMD_MASTER_RESP : KL_PROXY_CMD_SLAVER_RESP;
	resp_header.status = ret;
	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		presp_stream = NULL;
	}
	if(presp_stream == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create responsing stream", \
			__LINE__);
		delete pkg_msg_ptr;
		delete pvnode_balancer;
		return ENOMEM;
	}
	
	if((ret = presp_stream->stream_send(&resp_header, sizeof(resp_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"send responsing message's header failed, err: %s", \
			__LINE__, strerror(ret));
	}
	delete presp_stream;
	delete pkg_msg_ptr;
	delete pvnode_balancer;
	return ret;
}

int CProxyMsgParser::msg_device_report_handle(pkg_message* pkg_msg_ptr)
{
	//parse msg
	int ret, res;
	int i, vnode_count;
	CTimedStream *presp_stream;
	int nstorage_port;
	pstorage_info storage_info_ptr;
	device_info_ptr pdevice_info;
	pkg_header proxy_resp_header;
	vnode_list_unit pvnode_unit;

	//check whether the msg is legal
	if((pkg_msg_ptr->pkg_len - sizeof(storage_info) - 2) % \
		sizeof(vnode_list_unit) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"device beat-hearting msg is illegal", \
			__LINE__);
		delete pkg_msg_ptr;
		return -1;
	}

	try
	{
		presp_stream = new CTimedStream(pkg_msg_ptr->sock_stream_fd, false);
	}
	catch(std::bad_alloc)
	{
		presp_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create response stream for storage node beat-hearting msg", \
			__LINE__);
		delete pkg_msg_ptr;
		return ENOMEM;
	}
	catch(int errcode)
	{
		presp_stream = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"proxy server call CTimedStream constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		delete pkg_msg_ptr;
		return errcode;
	}

	vnode_count = (pkg_msg_ptr->pkg_len - sizeof(storage_info) - 2) / sizeof(vnode_list_unit);
	storage_info_ptr = (pstorage_info)(pkg_msg_ptr->pkg_ptr + 2);
	if(strcmp((char*)(storage_info_ptr->device_bind_ip), "0.0.0.0") == 0)
	{
		CInetAddr peer_addr;
		if(presp_stream->getpeeraddr(&peer_addr) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"proxy server report msg handle get perr address failed", \
				__LINE__);
			delete pkg_msg_ptr;
			return -1;
		}
		peer_addr.getipaddress((char*)(storage_info_ptr->device_bind_ip), KL_COMMON_IP_ADDR_LEN);
	}
	nstorage_port = CSERIALIZER::buff2int32(storage_info_ptr->device_bind_port);
	//get node info
	if((ret = g_pdevice_chg_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"rdlock device chg rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		delete presp_stream;
		delete pkg_msg_ptr;
		return ret;
	}
	pdevice_info = g_pdevice_container->get_node_by_addr((char*)(storage_info_ptr->device_bind_ip), \
		nstorage_port);
	if((ret = g_pdevice_chg_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock device chg rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		delete presp_stream;
		delete pkg_msg_ptr;
		return ret;
	}

	if(pdevice_info == NULL)
	{
		//proxy operation
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the storage node(ip: %s, port: %d) isn't logined", \
			__LINE__, (char*)(storage_info_ptr->device_bind_ip), nstorage_port);
		ret = -1;
		proxy_resp_header.cmd = g_bmaster_flag ? KL_PROXY_CMD_MASTER_RESP : \
			KL_PROXY_CMD_SLAVER_RESP;
		proxy_resp_header.status = -1;
		CSERIALIZER::long2buff(0, proxy_resp_header.pkg_len);
		if((res = presp_stream->stream_send(&proxy_resp_header, sizeof(pkg_header), \
			g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"proxy response stream send resp header failed, err: %s", \
				__LINE__, strerror(res));
			ret = res;
		}
		delete presp_stream;
		delete pkg_msg_ptr;
		return ret;
	}

	//update beat-hearting time
	pdevice_info->last_update_time = time(NULL);
	//merge device info
	if(g_bmaster_flag == true)
	{
		if((ret = master_do_device_merge(pdevice_info, (pvnode_list_unit)(pkg_msg_ptr->pkg_ptr + \
			sizeof(storage_info) + 2), vnode_count, presp_stream)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"master do device merge failed, errcode: %d", \
				__LINE__, ret);
			delete presp_stream;
			delete pkg_msg_ptr;
			return ret;
		}
	}
	else
	{
		if((ret = slaver_do_device_merge(pdevice_info, (pvnode_list_unit)(pkg_msg_ptr->pkg_ptr + \
			sizeof(pstorage_info) + 2), vnode_count)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"slaver do device merge failed, errcode: %d", \
				__LINE__, ret);
		}
		proxy_resp_header.cmd = KL_PROXY_CMD_SLAVER_RESP;
		CSERIALIZER::long2buff(0, proxy_resp_header.pkg_len);
		proxy_resp_header.status = ret;
		if((res = presp_stream->stream_send(&proxy_resp_header, sizeof(pkg_header), \
			g_ntimeout)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"proxy resp stream send response header failed, err: %s", \
				__LINE__, strerror(res));
			ret = res;
		}
	}
	delete presp_stream;
	delete pkg_msg_ptr;
	return ret;
}

int CProxyMsgParser::master_do_device_merge(device_info_ptr pdevice_info, \
	pvnode_list_unit pvnode_list, int vnode_count, CTimedStream *presp_stream)
{
	int ret;
	int64_t pkg_len;
	int i, nvnode_id;
	byte vnode_status;
	int64_t vnode_version;
	pkg_header resp_header;
	vnode_resp_unit resp_vnode;
	pvnode_resp_unit presp_body;
	vnode_info_ptr pproxy_vnode;
	sync_event vnode_sync_event;
	replica_info_ptr psync_replica;
	replica_info_ptr pproxy_replica;
	pvnode_list_unit pstorage_vnode;
	std::vector<int> device_vnode_list;
	std::vector<vnode_resp_unit> vnode_resp_list;

	//KL_SYS_INFOLOG("proxy node call master_do_device_merge");
	if((ret = g_pdevice_chg_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"wrlock device chg rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}
	device_vnode_list = pdevice_info->vnode_list;
	while(!pdevice_info->sync_list.empty())
	{
		vnode_sync_event = pdevice_info->sync_list.front();
		pdevice_info->sync_list.pop_front();
		if((ret = g_pdevice_chg_rwlock->unlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"unlock device_chg_rwlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
		for(i = 0; i < device_vnode_list.size(); i++)
		{
			if(vnode_sync_event.src_vnode_index == device_vnode_list[i])
				break;
		}
#ifdef _DEBUG
		assert(i < device_vnode_list.size());
#endif //__DEBUG
		device_vnode_list.erase(device_vnode_list.begin() + i);
		//push to syncing list
		pdevice_info->syncing_list.push_back(vnode_sync_event);

		pproxy_vnode = g_pvnode_container->get_vnode(vnode_sync_event.src_vnode_index);
		ret = g_pvnode_container->m_pvnode_rwlock->rdlock();
#ifdef _DEBUG
		assert(ret == 0);
#endif //_DEBUG
		pproxy_replica = pproxy_vnode->get_replica_info(pdevice_info);
		resp_vnode.vnode_status = pproxy_replica->replica_status;
		ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
		assert(ret == 0);
#endif //_DEBUG
		CSERIALIZER::int2buff(pproxy_vnode->vnode_id, resp_vnode.vnode_id);
		CSERIALIZER::int2buff(vnode_sync_event.pdest_node->nbind_port, resp_vnode.sync_dest_port);
		memcpy(resp_vnode.sync_dest_ip, vnode_sync_event.pdest_node->bind_ip, KL_COMMON_IP_ADDR_LEN);
		vnode_resp_list.push_back(resp_vnode);

		if((ret = g_pdevice_chg_rwlock->rdlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"rdlock device_chg_rwlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
	}
	if((ret = g_pdevice_chg_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock device_chg_rwlock failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}
	//KL_SYS_INFOLOG("proxy beat-hearting handle response sync event successfully");

	for(i = 0; i < vnode_count; i++)
	{
		int j;

		pstorage_vnode = (pvnode_list_unit)(pvnode_list + i);
#ifdef _DEBUG
		assert(pstorage_vnode != NULL);
#endif //_DEBUG
		nvnode_id = CSERIALIZER::buff2int32(pstorage_vnode->vnode_id);
		vnode_status = pstorage_vnode->vnode_status;
		vnode_version = CSERIALIZER::buff2int64(pstorage_vnode->vnode_version);

		for(j = 0; j < device_vnode_list.size(); j++)
		{
			if(nvnode_id == device_vnode_list[j])
				break;
		}
		//not found, no replica, so tell storage node to delete the vnode replica
		if(j == device_vnode_list.size())
		{
			CSERIALIZER::int2buff(nvnode_id, resp_vnode.vnode_id);
			resp_vnode.vnode_status = KL_REPLICA_STATUS_OFFLINE;
			vnode_resp_list.push_back(resp_vnode);
			continue;
		}
		device_vnode_list.erase(device_vnode_list.begin() + j);

		pproxy_vnode = g_pvnode_container->get_vnode(nvnode_id);
		if((ret = g_pvnode_container->m_pvnode_rwlock->wrlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"wrlock vnode container wrlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
		pproxy_replica = pproxy_vnode->get_replica_info(pdevice_info);
		memset(&resp_vnode, 0, sizeof(vnode_resp_unit));
		if(pproxy_replica->replica_status == KL_REPLICA_STATUS_COPY_SRC || \
			pproxy_replica->replica_status == KL_REPLICA_STATUS_MOVE_SRC)
		{
			ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
			assert(ret == 0);
#endif //_DEBUG
			for(j = 0; j < device_vnode_list.size(); j++)
			{
				if(nvnode_id == device_vnode_list[j])
					break;
			}
			if(j < device_vnode_list.size())
			{
				device_vnode_list.erase(device_vnode_list.begin() + j);
			}
			continue;
		}
		else if(pproxy_replica->replica_status == KL_REPLICA_STATUS_WAIT_SYNC)
		{
			ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
			assert(ret == 0);
#endif //_DEBUG
			CSERIALIZER::int2buff(nvnode_id, resp_vnode.vnode_id);
			resp_vnode.vnode_status = pproxy_replica->replica_status;
			vnode_resp_list.push_back(resp_vnode);
		}
		else
		{
			//need to sync
			if(vnode_version < pproxy_vnode->version)
			{
				if(pproxy_vnode->get_read_replica(&psync_replica) == -2)
				{
					KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
						"proxy beat-hearting handle get read replica failed", \
						__LINE__);
					g_pvnode_container->m_pvnode_rwlock->unlock();
					return -1;
				}
				//no active replica
				if(psync_replica == NULL)
				{
					pproxy_replica->replica_status = KL_REPLICA_STATUS_ONLINE;
					ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
					assert(ret == 0);
#endif //_DEBUG
					CSERIALIZER::int2buff(nvnode_id, resp_vnode.vnode_id);
					resp_vnode.vnode_status = KL_REPLICA_STATUS_ONLINE;
					vnode_resp_list.push_back(resp_vnode);
					continue;
				}
				//have active replica to sync
				pproxy_replica->replica_status = KL_REPLICA_STATUS_WAIT_SYNC;
				psync_replica->replica_status = KL_REPLICA_STATUS_COPY_SRC;
				ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
				assert(ret == 0);
#endif //_DEBUG
				vnode_sync_event.src_vnode_index = nvnode_id;
				vnode_sync_event.pdest_node = pdevice_info;

				ret = g_pdevice_chg_rwlock->wrlock();
#ifdef _DEBUG
				assert(ret == 0);
#endif //_DEBUG
				psync_replica->preplica->sync_list.push_back(vnode_sync_event);
				ret = g_pdevice_chg_rwlock->unlock();
#ifdef _DEBUG
				assert(ret == 0);
#endif //_DEBUG

				CSERIALIZER::int2buff(nvnode_id, resp_vnode.vnode_id);
				resp_vnode.vnode_status = KL_REPLICA_STATUS_WAIT_SYNC;
				vnode_resp_list.push_back(resp_vnode);
			}
			else
			{
				if(vnode_version > pproxy_vnode->version)
					pproxy_vnode->version = vnode_version;
				resp_vnode.vnode_status = pproxy_replica->replica_status;
				ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
				assert(ret == 0);
#endif //_DEBUG
				CSERIALIZER::int2buff(nvnode_id, resp_vnode.vnode_id);
				vnode_resp_list.push_back(resp_vnode);
			}
		}
	}
	//KL_SYS_INFOLOG("proxy beat-hearting handle response storage vnode info successfully");

	for(i = 0; i < device_vnode_list.size(); i++)
	{
		nvnode_id = device_vnode_list[i];
		ret = g_pvnode_container->m_pvnode_rwlock->rdlock();
#ifdef _DEBUG
		assert(ret == 0);
#endif //_DEBUG
		pproxy_vnode = g_pvnode_container->get_vnode(nvnode_id);
		pproxy_replica = pproxy_vnode->get_replica_info(pdevice_info);
		resp_vnode.vnode_status = pproxy_replica->replica_status;
		ret = g_pvnode_container->m_pvnode_rwlock->unlock();
#ifdef _DEBUG
		assert(ret == 0);
#endif //_DEBUG
		CSERIALIZER::int2buff(nvnode_id, resp_vnode.vnode_id);
		vnode_resp_list.push_back(resp_vnode);
	}
	//KL_SYS_INFOLOG("proxy beat-hearting handle response deleteing vnode successfully");

	pkg_len = vnode_resp_list.size() * sizeof(vnode_resp_unit);
	resp_header.cmd = KL_PROXY_CMD_MASTER_RESP;
	resp_header.status = 0;
	CSERIALIZER::long2buff(pkg_len, resp_header.pkg_len);
	if((ret = presp_stream->stream_send(&resp_header, sizeof(resp_header), g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"proxy response stream send response header failed, err: %s", \
			__LINE__, strerror(ret));
		return ret;
	}
	if(pkg_len == 0)
		return 0;
	try
	{
		presp_body = new vnode_resp_unit[vnode_resp_list.size()];
	}
	catch(std::bad_alloc)
	{
		presp_body = NULL;
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create response body", \
			__LINE__);
		return ENOMEM;
	}
	for(i = 0; i < vnode_resp_list.size(); i++)
	{
		memcpy(presp_body + i, &vnode_resp_list[i], sizeof(vnode_resp_unit));
	}
	if((ret = presp_stream->stream_send(presp_body, pkg_len, g_ntimeout)) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"proxy response stream send response body failed, err: %s", \
			__LINE__, strerror(ret));
		delete [] presp_body;
		return ret;
	}
	delete [] presp_body;
	return 0;
}

int CProxyMsgParser::slaver_do_device_merge(device_info_ptr pdevice_info,  \
	pvnode_list_unit pvnode_list, int vnode_count)
{
	int i, j, ret;
	int nvnode_id;
	byte vnode_status;
	int64_t vnode_version;
	vnode_info_ptr pproxy_vnode;
	replica_info_ptr pproxy_replica;
	pvnode_list_unit pstorage_vnode;
	std::vector<int> device_vnode_list;

	device_vnode_list = pdevice_info->vnode_list;
	for(i = 0; i < vnode_count; i++)
	{
		pstorage_vnode = (pvnode_list_unit)(pvnode_list + i);
		nvnode_id = CSERIALIZER::buff2int32(pstorage_vnode->vnode_id);
		vnode_version = CSERIALIZER::buff2int64(pstorage_vnode->vnode_version);
		vnode_status = pstorage_vnode->vnode_status;

		pproxy_vnode = g_pvnode_container->get_vnode(nvnode_id);
#ifdef _DEBUG
		assert(pproxy_vnode);
#endif //_DEBUG
		for(j = 0; j < device_vnode_list.size(); j++)
		{
			if(nvnode_id == device_vnode_list[j])
				break;
		}
		if((ret = g_pvnode_container->m_pvnode_rwlock->wrlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"wrlock vnode container rwlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
		pproxy_vnode->version = vnode_version;
		pproxy_replica = pproxy_vnode->get_replica_info(pdevice_info);
		//not found replica, so add the replica info
		if(j == device_vnode_list.size())
		{
			try
			{
				pproxy_replica = new replica_info();
			}
			catch(std::bad_alloc)
			{
				pproxy_replica = NULL;
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"no more memory to create replica for vnode(vnode index: %d)", \
					__LINE__, nvnode_id);
				return ENOMEM;
			}
			pdevice_info->vnode_list.push_back(nvnode_id);
			pproxy_replica->replica_status = vnode_status;
			pproxy_replica->preplica = pdevice_info;
			pproxy_vnode->replica_list.push_back(pproxy_replica);
		}
		else //found the replica, so update it
		{
			pproxy_replica = pproxy_vnode->get_replica_info(pdevice_info);
			pproxy_replica->replica_status = vnode_status;
			device_vnode_list.erase(device_vnode_list.begin() + j);
		}
		if((ret = g_pvnode_container->m_pvnode_rwlock->unlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"unlock vnode container rwlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
	}
	//if device_vnode_list isn't empty, indicating that the vnode id in device_vnode_list
	//isn't exist in this device, so we need delete every vnode replica info of the device
	for(i = 0; i < device_vnode_list.size(); i++)
	{
		nvnode_id = device_vnode_list[i];
		pproxy_vnode = g_pvnode_container->get_vnode(nvnode_id);
		if((ret = g_pvnode_container->m_pvnode_rwlock->wrlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"wrlock vnode container rwlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
		pproxy_vnode->destroy_replica_info(pdevice_info);
		if((ret = g_pvnode_container->m_pvnode_rwlock->unlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"unlock vnode container rwlock failed, err: %s", \
				__LINE__, strerror(ret));
			return ret;
		}
	}
	return 0;
}