#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "rwlock.h"
#include "node_info.h"
#include "common_types.h"
#include "proxy_protocol.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CDeviceContainer::CDeviceContainer() : \
	m_node_count(0), m_container_size(KL_PROXY_CONTAINER_SIZE)
{
	m_pnode_container = (device_info_ptr*)malloc(sizeof(device_info_ptr) * \
		KL_PROXY_CONTAINER_SIZE);
	if(m_pnode_container == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create device container, err: %s", \
			__LINE__, strerror(errno));
		return;
	}
	memset(m_pnode_container, 0, sizeof(device_info_ptr) * KL_PROXY_CONTAINER_SIZE);

	m_pnode_rwlock = new CRWLock();
	if(m_pnode_rwlock == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create rwlock object", \
			__LINE__);
		return;
	}
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CDeviceContainer constructor successfully");
#endif //_DEBUG
}

CDeviceContainer::~CDeviceContainer()
{
	int node_curr;
	for(node_curr = 0; node_curr < m_node_count; node_curr++)
	{
		if(m_pnode_container[node_curr] != NULL)
		{
			delete m_pnode_container[node_curr];
			m_pnode_container[node_curr] = NULL;
		}
	}
	free(m_pnode_container);

	if(m_pnode_rwlock != NULL)
	{
		delete m_pnode_rwlock;
		m_pnode_rwlock = NULL;
	}
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CDeviceContainer destructor successfully");
#endif //_DEBUG
}

int CDeviceContainer::node_diff_cmp(device_info_ptr ptr1, device_info_ptr ptr2)
{
#ifdef _DEBUG
	assert(ptr1 && ptr2);
#endif //_DEBUG
	if((memcmp(ptr1->bind_ip, ptr2->bind_ip, KL_COMMON_IP_ADDR_LEN) == 0) \
		&& (ptr1->bind_port == ptr2->bind_port))
		return 0;
	return -1;
}

int CDeviceContainer::node_vol_cmp(const void *ptr1, const void *ptr2)
{
	float node1_volume;
	float node2_volume;
	device_info_ptr node_ptr1 = (device_info_ptr)ptr1;
	device_info_ptr node_ptr2 = (device_info_ptr)ptr2;
#ifdef _DEBUG
	assert(node_ptr1 && node_ptr2);
#endif //_DEBUG
	if(node_ptr1->weight <= 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the device's(node id: %d) weight is illegal", \
			__LINE__, node_ptr1->device_id);
		node_ptr1->weight = 1;
	}
	if(node_ptr2->weight <= 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the device's(node id: %d) weight is illegal", \
			__LINE__, node_ptr2->device_id);
		node_ptr2->weight = 1;
	}
	node1_volume = (float)(node_ptr1->vnode_count) / (float)(node_ptr1->weight);
	node2_volume = (float)(node_ptr2->vnode_count) / (float)(node_ptr2->weight);

	if(node1_volume > node2_volume)
		return 1;
	else if(node1_volume == node2_volume)
		return 0;
	else
		return -1;
}

int CDeviceContainer::delete_node(device_info_ptr pdevice_info)
{
	int node_curr;
	int res;
	device_info_ptr pdevice_curr;
	device_info_ptr temp_ptr;

	if((res = m_pnode_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's wrlock failed, err: %s", \
			__LINE__, strerror(res));
		return -1;
	}

	for(node_curr = 0; node_curr < m_node_count; node_curr++)
	{
		if(node_diff_cmp(pdevice_info, \
			m_pnode_container[node_curr]) == 0)
			break;
	}

	//swap node
	temp_ptr = m_pnode_container[m_node_count - 1];
	m_pnode_container[m_node_count - 1] = m_pnode_container[node_curr];
	m_pnode_container[node_curr] = temp_ptr;
	m_pnode_container[node_curr]->device_id = node_curr;
	if(m_pnode_container[m_node_count - 1] != NULL)
	{
		delete m_pnode_container[m_node_count - 1];
		m_pnode_container[m_node_count - 1] = NULL;
		m_node_count--;

		if((res = m_pnode_rwlock->unlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call rwlock's unlock failed, err: %s", \
				__LINE__, strerror(res));
			return -1;
		}
		return 0;
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"delete node failed, err: the node is null", \
		__LINE__);
	m_node_count--;

	if((res = m_pnode_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's unlock failed, err: %s", \
			__LINE__, strerror(res));
	}
	return -1;
}

int CDeviceContainer::add_node(device_info_ptr pdevice_info)
{
	int res;
	int old_container_size;

	if((res = m_pnode_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's wrlock failed, err: %s", \
			__LINE__, strerror(res));
		return -1;
	}

	if(m_node_count == m_container_size)
	{
		old_container_size = m_container_size;
		m_pnode_container = (device_info_ptr*)realloc(m_pnode_container, \
			m_container_size + KL_PROXY_CONTAINER_SIZE);
		if(m_pnode_container == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to realloc node container with new size: %d", \
				__LINE__, m_container_size + KL_PROXY_CONTAINER_SIZE);
			m_node_count = 0;
			m_container_size = 0;

			if((res = m_pnode_rwlock->unlock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"call rwlock's unlock failed, err: %s", \
					__LINE__, strerror(res));
				return -1;
			}
			return ENOMEM;
		}

		m_container_size += KL_PROXY_CONTAINER_SIZE;
		memset(m_pnode_container + old_container_size, 0, KL_PROXY_CONTAINER_SIZE);
	}

	m_pnode_container[m_node_count++] = pdevice_info;

	if((res = m_pnode_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's unlock failed, err: %s", \
			__LINE__, strerror(res));
		return -1;
	}
	return 0;
}

int CDeviceContainer::merge_node(device_info_ptr pdevice_info)
{
	int res;
	int node_curr;

	if((res = m_pnode_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's rwlock failed, err: %s", \
			__LINE__, strerror(res));
		return -1;
	}

	for(node_curr = 0; node_curr < m_node_count; node_curr++)
	{
		if(node_diff_cmp(pdevice_info, \
			m_pnode_container[node_curr]) == 0)
			break;
	}
	if(node_curr == m_node_count)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"merge node failed, the node isn't exist in node container", \
			__LINE__);

		if((res = m_pnode_rwlock->unlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"call rwlock's unlock failed, err: %s", \
				__LINE__, strerror(res));
		}
		return -1;
	}
	memcpy(m_pnode_container[node_curr], pdevice_info, sizeof(device_info));

	if((res = m_pnode_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's unlock failed, err: %s", \
			__LINE__, strerror(res));
		return -1;
	}
	return 0;
}

device_info_ptr CDeviceContainer::get_node(int *expected_zones, int count)
{
	int res;
	int zone_curr;
	device_info_ptr pdevice_curr;

	if((res = m_pnode_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's rwlock failed, err: %s", \
			__LINE__, strerror(res));
		return NULL;
	}

	qsort((void*)m_pnode_container, m_node_count, \
		sizeof(device_info_ptr), node_vol_cmp);

	pdevice_curr = m_pnode_container[0];
	while(pdevice_curr <= m_pnode_container[m_node_count - 1])
	{
		for(zone_curr = 0; zone_curr < count; zone_curr++)
		{
			if(pdevice_curr->zone_id == expected_zones[zone_curr])
			{
				pdevice_curr++;
				break;
			}
			else
			{
				if((res = m_pnode_rwlock->unlock()) != 0)
				{
					KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
						"call rwlock's unlock failed, err: %s", \
						__LINE__, strerror(res));
					return NULL;
				}
				return pdevice_curr;
			}
		}
	}

	KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
		"no device can meet the conditions", \
		__LINE__);
	if((res = m_pnode_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call rwlock's unlock failed, err: %s", \
			__LINE__, strerror(res));
	}
	return NULL;
}

int CDeviceContainer::get_node_count()
{
	return m_node_count;
}

CVnodeContainer::CVnodeContainer(int vnode_count, int replica_count) : \
	m_vnode_count(vnode_count), m_replica_count(replica_count)
{
	int vnode_curr;

	m_pvnode_container = new vnode_info_ptr[m_vnode_count];
	if(m_pvnode_container == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode container", \
			__LINE__);
		return;
	}

	for(vnode_curr = 0; vnode_curr < m_vnode_count; vnode_curr++)
	{
		m_pvnode_container[vnode_curr] = new vnode_info();
		if(m_pvnode_container[vnode_curr] == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create a vnode info object");
			return;
		}
	}

	m_pvnode_rwlock = new CRWLock();
	if(m_pvnode_rwlock == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode rwlock", \
			__LINE__);
		return;
	}
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CVnodeContainer constructor successfully");
#endif //_DEBUG
}

CVnodeContainer::~CVnodeContainer()
{
	int vnode_curr;

	if(m_pvnode_container == NULL)
		return;

	for(vnode_curr = 0; vnode_curr < m_vnode_count; vnode_curr++)
	{
		if(m_pvnode_container[vnode_curr] != NULL)
		{
			if(m_pvnode_container[vnode_curr]->replica_list != NULL)
			{
				delete m_pvnode_container[vnode_curr]->replica_list;
				m_pvnode_container[vnode_curr]->replica_list = NULL;
			}
			delete m_pvnode_container[vnode_curr];
			m_pvnode_container[vnode_curr] = NULL;
		}
	}
	delete m_pvnode_container;
	m_pvnode_container = NULL;

	if(m_pvnode_rwlock != NULL)
	{
		delete m_pvnode_rwlock;
		m_pvnode_rwlock = NULL;
	}
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CVnodeContainer destrcutor successfully");
#endif
}

int CVnodeContainer::initilize()
{
	int vnode_curr;
	int replica_curr;
	replica_info_ptr preplica_list;
#ifdef _DEBUG
	assert(m_pvnode_container);
#endif //_DEBUG
	for(vnode_curr = 0; vnode_curr < m_vnode_count; vnode_curr++)
	{
		if(m_pvnode_container[vnode_curr] == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"vnode_info(vnod_id = %d) is null", \
				__LINE__, vnode_curr);
			return -1;
		}

		m_pvnode_container[vnode_curr]->vnode_id = vnode_curr;
		m_pvnode_container[vnode_curr]->version = 0;
		m_pvnode_container[vnode_curr]->rd_req_robin = 0;
		m_pvnode_container[vnode_curr]->wr_req_robin = 0;
		preplica_list = new replica_info[m_replica_count];
		if(preplica_list == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create a replica list(list size: %d) to " \
				"vnode(vnode_id: %d)", \
				__LINE__, m_replica_count, vnode_curr);
			return ENOMEM;
		}
		for(replica_curr = 0; replica_curr < m_replica_count; replica_curr++)
		{
			preplica_list[replica_curr].replica_status = KL_DEVICE_STATUS_INIT;
			preplica_list[replica_curr].preplica = NULL;
		}
		m_pvnode_container[vnode_curr]->replica_list = preplica_list;
	}
	return 0;
}

int CVnodeContainer::get_replica_count()
{
	return m_replica_count;
}

vnode_info_ptr CVnodeContainer::get_vnode(int index)
{
	int res;
	vnode_info_ptr pvnode_info;

	if((res = m_pvnode_rwlock->rdlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"rdlock m_pvnode_rwlock failed, err: %s", \
			__LINE__, strerror(res));
		return NULL;
	}
	pvnode_info = m_pvnode_container[index];
	if((res = m_pvnode_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock m_pvnode_rwlock failed, err: %s", \
			__LINE__, strerror(res));
		return NULL;
	}
	if(pvnode_info == NULL || pvnode_info->vnode_id != index)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the vnode info(index: %d) is illegal", \
			__LINE__, index);
		return NULL;
	}
	return pvnode_info;
}

int CVnodeContainer::merge_vnode(vnode_info_ptr pvnode_info)
{
	int res;
	int index;
#ifdef _DEBUG
	assert(pvnode_info);
#endif //_DEBUG
	if((res = m_pvnode_rwlock->wrlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"wrlock m_pvnode_rwlock failed, err: %s", \
			__LINE__, strerror(res));
		return res;
	}
	index = pvnode_info->vnode_id;
	if(index < 0 || index >= m_vnode_count)
	{
		if((res = m_pvnode_rwlock->unlock()) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"unlock m_pvnode_rwlock failed, err: %s", \
				__LINE__, strerror(res));
			return res;
		}
		return -1;
	}
	memcpy(m_pvnode_container[index], pvnode_info, sizeof(vnode_info));
	if((res = m_pvnode_rwlock->unlock()) != 0)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"unlock m_pvnode_rwlock failed, err: %s", \
			__LINE__, strerror(res));
		return res;
	}
	return 0;
}

replica_info_ptr CVnodeContainer::get_write_replica(int index)
{

}