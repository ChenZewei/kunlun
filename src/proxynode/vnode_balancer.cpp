#include <new>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "rwlock.h"
#include "proxy_global.h"
#include "node_container.h"
#include "vnode_balancer.h"
#include "proxy_protocol.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CVnodeBalancer::CVnodeBalancer()
{

}

CVnodeBalancer::~CVnodeBalancer()
{

}

int CVnodeBalancer::master_do_copy_vnode(device_info_ptr pdevice_info)
{
	int res;
	int i;
	sync_event device_sync_event;
	vnode_info_ptr pvnode_info;
	replica_info_ptr pjoin_replica;
	replica_info_ptr psrc_sync_replica;

#ifdef _DEBUG
	assert(pdevice_info);
#endif //_DEBUG
	//every vnode has not enough replicas
	for(i = 0; i < g_pvnode_container->get_vnode_count(); i++)
	{
		pvnode_info = g_pvnode_container->get_vnode(i);
		if(pvnode_info == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get vnode info falied, vnode index: %d", \
				__LINE__, i);
			return -1;
		}

		//if not in different zones, refuse to join
		if(pvnode_info->is_diff_zones(pdevice_info) == false)
		{
			KL_SYS_NOTICELOG("vnode(index: %d) can't move to storagenode(ip: %s, port: %d), " \
				"err: vnode's replicas can't exist in the same zone", \
				i, pdevice_info->bind_ip, pdevice_info->nbind_port);
			continue;
		}

		try
		{
			pjoin_replica = new replica_info();
		}
		catch(std::bad_alloc)
		{
			pjoin_replica = NULL;
		}
		if(pjoin_replica == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create replica info for vnode(vnode_id: %d)", \
				__LINE__, i);
			return ENOMEM;
		}

		pjoin_replica->preplica = pdevice_info;
		pjoin_replica->replica_status = KL_REPLICA_STATUS_ONLINE;
		//the first replica
		if(pvnode_info->replica_list.size() == 0)
		{
			pjoin_replica->replica_status = KL_REPLICA_STATUS_ACTIVE;
		}
		else
		{
			//choose a syncing src replica to sync the joining replica
			if((res = g_pvnode_container->m_pvnode_rwlock->wrlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"wrlock g_pvnode_container failed, err: %s", \
					__LINE__, strerror(res));
				delete pjoin_replica;
				pjoin_replica = NULL;
				return -1;
			}
			//if vnode container has active replica to sync data, we choose a replica and
			//set sync event in it's sync queue. when the next beat heart coming, the master
			//will return synchronized event to the beat-hearting storage node.
			//if vnode container has no active replica to sync data, we just ignore the sync
			//event temporarily,the sync event will be completed by beat heart operation when
			//the vnode has a active replica.
			if((res = pvnode_info->get_read_replica(&psrc_sync_replica)) == -2)
			{
				KL_SYS_ERRORLOG("flie: "__FILE__", line: %d, " \
					"get sync src replica failed, device join failed", \
					__LINE__);
				g_pvnode_container->m_pvnode_rwlock->unlock();
				delete pjoin_replica;
				pjoin_replica = NULL;
				return -1;
			}
			//set the synchronized event to src replica's sync queue
			if(psrc_sync_replica != NULL)
			{
				pjoin_replica->replica_status = KL_REPLICA_STATUS_WAIT_SYNC;
				psrc_sync_replica->replica_status = KL_REPLICA_STATUS_COPY_SRC;
				device_sync_event.src_vnode_index = i;
				device_sync_event.pdest_node = pdevice_info;
				psrc_sync_replica->preplica->sync_list.push_back(device_sync_event);
			}

			if((res = g_pvnode_container->m_pvnode_rwlock->unlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"unlock g_pvnode_container failed, err: %s", \
					__LINE__, strerror(res));
				delete pjoin_replica;
				pjoin_replica = NULL;
				return -1;
			}
		}
		pvnode_info->replica_list.push_back(pjoin_replica);
		pdevice_info->vnode_list.push_back(i);
	}
	pdevice_info->last_update_time = time(NULL);
	g_pdevice_container->add_node(pdevice_info);
	return 0;
}

int CVnodeBalancer::master_do_move_vnode(device_info_ptr pdevice_info)
{
#define goto_choose_another_vnode() \
	vnode_list.erase(vnode_list.begin() + i); \
	continue

	int ret;
	int i, j;
	int total_count;
	int total_weight;
	int node_index;
	int vnode_index;
	int move_count;
	int new_avg_count;
	int curr_move_count;
	sync_event device_sync_event;
	std::vector<int> vnode_list;
	device_info_ptr pcurr_device;
	vnode_info_ptr pcurr_vnode;
	replica_info_ptr pjoin_replica;
	replica_info_ptr psrc_sync_replica;

#ifdef _DEBUG
	assert(pdevice_info);
#endif //_DEBUG

	if(g_pdevice_container->get_total_replica_count(&total_count, &total_weight) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"get total replica count failed", \
			__LINE__);
		return -1;
	}

	srand(time(NULL));
	total_weight += pdevice_info->weight;
	new_avg_count = total_count / total_weight;
	for(node_index = 0; node_index < g_pdevice_container->get_node_count(); \
		node_index++)
	{
		pcurr_device = g_pdevice_container->get_node(node_index);
		if(pcurr_device == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"get node info failed in node container", \
				__LINE__);
			return -1;
		}

		move_count = pcurr_device->vnode_list.size() - (new_avg_count * pcurr_device->weight);
		if(move_count <= 0)
		{
#ifdef _DEBUG
			KL_SYS_DEBUGLOG("file: "__FILE__", line: %d, " \
				"the current deivce(device ip: %s, port: %d) vnode size less than the avg count", \
				__LINE__, pcurr_device->bind_ip, pcurr_device->nbind_port);
#endif //_DEBUG
			continue; //not to move
		}

		curr_move_count = 0;
		vnode_list = pcurr_device->vnode_list;
		//choose move_count vnode moving to the joind device
		while(vnode_list.size() > 0)
		{
			i = rand() % vnode_list.size();
			vnode_index = pcurr_device->vnode_list[i];
			//check whether the replica's status is KL_DEVICE_STATUS_ACTIVE
			//only the replica which status is KL_DEVICE_STATUS_ACTIVE can be moved
			pcurr_vnode = g_pvnode_container->get_vnode(vnode_index);
			if(pcurr_vnode == NULL)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"get vnode info failed in vnode container", \
					__LINE__);
				return -1;
			}
			if((psrc_sync_replica = pcurr_vnode->get_replica_info(pcurr_device)) == NULL)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"the vnode(vnode index: %d) doesn't have the current device's " \
					"replica info(device ip: %s, port: %d)", \
					__LINE__, vnode_index, pcurr_device->bind_ip, pcurr_device->nbind_port);
				return -1;
			}
			if(psrc_sync_replica->replica_status != KL_REPLICA_STATUS_ACTIVE)
			{
#ifdef _DEBUG
				KL_SYS_DEBUGLOG("file: "__FILE__", line: %d, " \
					"the status of vnode(vnode index: %d)'s replica isn't " \
					"KL_DEVICE_STATUS_ACTIVE, choose another vnode, repica(ip: %s, port: %d)", \
					__LINE__, vnode_index, pcurr_device->bind_ip, pcurr_device->nbind_port);
#endif //_DEBUG
				goto_choose_another_vnode();
			}
			//check whether the vnode index in move list
			for(j = 0; j < pdevice_info->vnode_list.size(); j++)
			{
				if(vnode_index == pdevice_info->vnode_list[j])
					break;
			}
			if(j < pdevice_info->vnode_list.size())
			{
				//the vnode index in move list, so we erase it and choose another one
#ifdef _DEBUG
				KL_SYS_DEBUGLOG("file: "__FILE__", line: %d, " \
					"the vnode(vnode index: %d) has been moved to the joining " \
					"device(ip: %s, port: %d)", \
					__LINE__, vnode_index, pdevice_info->bind_ip, pdevice_info->nbind_port);
#endif //_DEBUG
				goto_choose_another_vnode(); //to choose another vnode
			}

			//the vnode hasn't been moved to the joining device
			//check the zone condition
			if((pcurr_device->zone_id != pdevice_info->zone_id) && \
				(pcurr_vnode->is_diff_zones(pdevice_info) == false))
			{
#ifdef _DEBUG
				KL_SYS_DEBUGLOG("file: "__FILE__", line: %d, " \
					"the vnode(vnode index: %d) can't move to the joining " \
					"device(ip: %s, port: %d), reason: replica can't locate in the same zone", \
					__LINE__, vnode_index, pdevice_info->bind_ip, pdevice_info->nbind_port);
#endif //_DEBUG
				goto_choose_another_vnode();
			}
			//check whether the src syncing replica's status is changed
			if((ret = g_pvnode_container->m_pvnode_rwlock->wrlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"rdlock the device container failed, err: %s", \
					__LINE__, strerror(ret));
				return -1;
			}
			if(psrc_sync_replica->replica_status != KL_REPLICA_STATUS_ACTIVE)
			{
				if((ret = g_pvnode_container->m_pvnode_rwlock->unlock()) != 0)
				{
					KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
						"unlock the device container failed, err: %s", \
						__LINE__, strerror(ret));
					return -1;
				}
				goto_choose_another_vnode();
			}
			psrc_sync_replica->replica_status = KL_REPLICA_STATUS_MOVE_SRC;
			if((ret = g_pvnode_container->m_pvnode_rwlock->unlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"unlock the device container failed, err: %s", \
					__LINE__, strerror(ret));
				return -1;
			}

			//can move the vnode
			try
			{
				pjoin_replica = new replica_info();
			}catch(std::bad_alloc)
			{
				pjoin_replica = NULL;
			}
			if(pjoin_replica = NULL)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"no more memroy to create replica info for vnode(vnode index: %d)", \
					__LINE__, vnode_index);
				return ENOMEM;
			}

			pjoin_replica->replica_status = KL_REPLICA_STATUS_ONLINE;
			pjoin_replica->preplica = pdevice_info;
			//wait a src synchronized storagenode to sync data
			pjoin_replica->replica_status = KL_REPLICA_STATUS_WAIT_SYNC;
			//produce a synchronized event
			device_sync_event.src_vnode_index = vnode_index;
			device_sync_event.pdest_node = pdevice_info;
			if((ret = g_pvnode_container->m_pvnode_rwlock->wrlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"wrlock the device container failed, err: %s", \
					__LINE__, strerror(ret));
				return -1;
			}
			psrc_sync_replica->preplica->sync_list.push_back(device_sync_event);
			if((ret = g_pvnode_container->m_pvnode_rwlock->unlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"unlock the device container failed, err: %s", \
					__LINE__, strerror(ret));
				return -1;
			}
			pcurr_vnode->replica_list.push_back(pjoin_replica);
			pdevice_info->vnode_list.push_back(vnode_index);
			//check whether the move count is done
			curr_move_count++;
			if(curr_move_count >= move_count)
				break;
			goto_choose_another_vnode();
		}
	}
	pdevice_info->last_update_time = time(NULL);
	g_pdevice_container->add_node(pdevice_info);
	return 0;
}

int CVnodeBalancer::slaver_do_device_join(device_info_ptr pdevice_info)
{
	pdevice_info->last_update_time = time(NULL);
	g_pdevice_container->add_node(pdevice_info);
	return 0;
}