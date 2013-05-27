#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "node_info.h"
#include "proxy_protocol.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

int device_info::erase_vnode(int vnode_index)
{
	int i;
	for(i = 0; i < vnode_list.size(); i++)
	{
		if(vnode_index == vnode_list[i])
			break;
	}
	if(i == vnode_list.size())
		return -1;
	vnode_list.erase(vnode_list.begin() + i);
	return 0;
}

sync_event device_info::pop_syncing_vnode(int vnode_index)
{
	sync_event se;
	sync_iter_t sync_iter;

	se.src_vnode_index = 0;
	se.pdest_node = NULL;
	for(sync_iter = syncing_list.begin(); sync_iter != syncing_list.end(); \
		sync_iter++)
	{
		if((*sync_iter).src_vnode_index == vnode_index)
		{
			se = *sync_iter;
			syncing_list.erase(sync_iter);
			break;
		}
	}
	return se;
}

vnode_info::~vnode_info()
{
	while(!replica_list.empty())
	{
		delete replica_list.front();
		replica_list.pop_front();
	}
}

int vnode_info::get_read_replica(replica_info_ptr *ppreplica)
{
	int i, j;
	replica_iter_t replica_iter;
	replica_info_ptr preplica_info;

	if(replica_list.empty())
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"get vnode read replica failed, err: the vnode has no replica", \
			__LINE__);
		*ppreplica = NULL;
		return -2;
	}
	for(i = 0; i < replica_list.size(); i++)
	{
		if(rd_req_robin >= replica_list.size())
		{
			rd_req_robin = 0;
		}

		replica_iter = replica_list.begin();
		for(j = 0; j < rd_req_robin; j++)
		{
			replica_iter++;
		}
		preplica_info = *replica_iter;
		rd_req_robin++;
		if(preplica_info->replica_status == KL_REPLICA_STATUS_ACTIVE)
		{
			*ppreplica = preplica_info;
			return 0;
		}
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"get vnode read replica failed, err: the vnode(vnode_id: %d) has no active replica", \
		__LINE__, vnode_id);
	*ppreplica = NULL;
	return -1;
}

int vnode_info::get_write_replica(replica_info_ptr *ppreplica)
{
	int i, j;
	replica_iter_t replica_iter;
	replica_info_ptr preplica_info;

	if(replica_list.size() == 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"get vnode write replica failed, err: the vnode has no replica", \
			__LINE__);
		*ppreplica = NULL;
		return -2;
	}

	for(i = 0; i < replica_list.size(); i++)
	{
		if(wr_req_robin >= replica_list.size())
		{
			wr_req_robin = 0;
		}

		replica_iter = replica_list.begin();
		for(j = 0; j < rd_req_robin; j++)
		{
			replica_iter++;
		}
		preplica_info = *replica_iter;
		wr_req_robin++;
		if(preplica_info->replica_status == KL_REPLICA_STATUS_ACTIVE)
		{
			*ppreplica = preplica_info;
			return 0;
		}
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"get vnode write replica failed, err: the vnode has no active replica", \
		__LINE__);
	*ppreplica = NULL;
	return -1;
}

replica_info_ptr vnode_info::get_replica_info(device_info_ptr preplica)
{
	replica_iter_t replica_iter;

#ifdef _DEBUG
	assert(preplica);
#endif //_DEBUG

	for(replica_iter = replica_list.begin(); replica_iter != replica_list.end(); \
		replica_iter++)
	{
		if(preplica == (*replica_iter)->preplica)
			break;
	}

	if(replica_iter != replica_list.end())
		return *replica_iter;
	return NULL;
}

bool vnode_info::is_diff_zones(device_info_ptr pjoin_device)
{
	replica_iter_t replica_iter;

	for(replica_iter = replica_list.begin(); replica_iter != replica_list.end(); \
		replica_iter++)
	{
		if(pjoin_device->zone_id == (*replica_iter)->preplica->zone_id)
			break;
	}
	if(replica_iter != replica_list.end())
		return false;
	return true;
}

int vnode_info::destroy_replica_info(device_info_ptr preplica)
{
	replica_iter_t replica_iter;
	
	for(replica_iter = replica_list.begin(); replica_iter != replica_list.end(); \
		replica_iter++)
	{
		if((*replica_iter)->preplica == preplica)
			break;
	}
	if(replica_iter != replica_list.end())
	{
		delete *replica_iter;
		replica_list.erase(replica_iter);
	}
	return 0;
}