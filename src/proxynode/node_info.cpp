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

vnode_info::~vnode_info()
{
	int i;
	for(i = 0; i < replica_list.size(); i++)
	{
		if(replica_list[i] != NULL)
		{
			delete replica_list[i];
			replica_list[i] = NULL;
		}
	}
}

int vnode_info::get_read_replica(replica_info_ptr *ppreplica)
{
	int i;
	replica_info_ptr preplica_info;

	if(replica_list.size() == 0)
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

		preplica_info = replica_list.at(rd_req_robin);
		rd_req_robin++;
		if(preplica_info->replica_status == KL_REPLICA_STATUS_ACTIVE)
		{
			*ppreplica = preplica_info;
			return 0;
		}
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"get vnode read replica failed, err: the vnode has no active replica", \
		__LINE__);
	*ppreplica = NULL;
	return -1;
}

int vnode_info::get_write_replica(replica_info_ptr *ppreplica)
{
	int i;
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

		preplica_info = replica_list.at(wr_req_robin);
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
	int i;

#ifdef _DEBUG
	assert(preplica);
#endif //_DEBUG

	for(i = 0; i < replica_list.size(); i++)
	{
		if(preplica == replica_list[i]->preplica)
			break;
	}

	if(i < replica_list.size())
		return replica_list[i];
	return NULL;
}

bool vnode_info::is_diff_zones(device_info_ptr pjoin_device)
{
	int replica_curr;

	for(replica_curr = 0; replica_curr < replica_list.size(); \
		replica_curr++)
	{
		if(pjoin_device->zone_id == \
			replica_list[replica_curr]->preplica->zone_id)
			break;
	}
	if(replica_curr != replica_list.size())
		return false;
	return true;
}

int vnode_info::destroy_replica_info(device_info_ptr preplica)
{
	int i;
	
	for(i = 0; i < replica_list.size(); i++)
	{
		if(replica_list[i]->preplica == preplica)
			break;
	}
	if(i < replica_list.size())
	{
		delete replica_list[i];
		replica_list.erase(replica_list.begin() + i);
	}
	return 0;
}