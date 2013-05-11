#include <new>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "rwlock.h"
#include "common_types.h"
#include "node_container.h"
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
		throw ENOMEM;
	}
	memset(m_pnode_container, 0, sizeof(device_info_ptr) * KL_PROXY_CONTAINER_SIZE);

	try
	{
		m_pnode_rwlock = new CRWLock();
	}
	catch(std::bad_alloc)
	{
		m_pnode_rwlock = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CRWLock contructor failed, err: %s", \
			__LINE__, strerror(errcode));
		throw errcode;
	}
	if(m_pnode_rwlock == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create rwlock object", \
			__LINE__);
		throw ENOMEM;
	}
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
}

int CDeviceContainer::node_diff_cmp(device_info_ptr ptr1, device_info_ptr ptr2)
{
#ifdef _DEBUG
	assert(ptr1 && ptr2);
#endif //_DEBUG
	if((memcmp(ptr1->bind_ip, ptr2->bind_ip, KL_COMMON_IP_ADDR_LEN) == 0) \
		&& (ptr1->nbind_port == ptr2->nbind_port))
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
	node1_volume = (float)(node_ptr1->vnode_list.size()) / (float)(node_ptr1->weight);
	node2_volume = (float)(node_ptr2->vnode_list.size()) / (float)(node_ptr2->weight);

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
		return 0;
	}
	KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
		"delete node failed, err: the node is null", \
		__LINE__);
	m_node_count--;
	return -1;
}

int CDeviceContainer::add_node(device_info_ptr pdevice_info)
{
	int res;
	int old_container_size;

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

			return ENOMEM;
		}

		m_container_size += KL_PROXY_CONTAINER_SIZE;
		memset(m_pnode_container + old_container_size, 0, KL_PROXY_CONTAINER_SIZE);
	}

	m_pnode_container[m_node_count++] = pdevice_info;
	return 0;
}

int CDeviceContainer::merge_node(device_info_ptr pdevice_info)
{
	int res;
	int node_curr;

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

		return -1;
	}
	memcpy(m_pnode_container[node_curr], pdevice_info, sizeof(device_info));

	return 0;
}

device_info_ptr CDeviceContainer::get_node(int *expected_zones, int count)
{
	int res;
	int zone_curr;
	device_info_ptr pdevice_curr;

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
		}
		if(zone_curr == count)
			return pdevice_curr;
	}

	KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
		"no device can meet the conditions", \
		__LINE__);
	return NULL;
}

device_info_ptr CDeviceContainer::get_node(int index)
{
	return m_pnode_container[index];
}

device_info_ptr CDeviceContainer::get_node_by_addr(char *pdevice_ip, int nport)
{
	int i;
	device_info dest_device;
	
	memset(dest_device.bind_ip, 0, KL_COMMON_IP_ADDR_LEN);
	strcpy(dest_device.bind_ip, pdevice_ip);
	dest_device.nbind_port = nport;
	for(i = 0; i < m_node_count; i++)
	{
		if(node_diff_cmp(&dest_device, m_pnode_container[i]) == 0)
			return m_pnode_container[i];
	}
	return NULL;
}

int CDeviceContainer::get_node_count()
{
	return m_node_count;
}

int CDeviceContainer::get_total_replica_count(int *ptotal_count, \
	int *ptotal_weight)
{
	int i;
	int total_count;
	int total_weight;
	device_info_ptr pcurr_device;
	
	total_count = 0;
	total_weight = 0;
	for(i = 0; i < m_node_count; i++)
	{
		pcurr_device = m_pnode_container[i];
		if(pcurr_device == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"the device info(index: %d) is illegal(null)", \
				__LINE__, i);
			return -1;
		}

		total_count += pcurr_device->vnode_list.size();
		total_weight += pcurr_device->weight;
	}
	*ptotal_count = total_count;
	*ptotal_weight = total_weight;
	return 0;
}

#ifdef _DEBUG
int CDeviceContainer::putout_vnode_count()
{
	int i;
	for(i = 0; i < m_node_count; i++)
	{
		KL_SYS_DEBUGLOG("device(bind_ip: %s, bind_port: %d), vnode count: %d", \
			m_pnode_container[i]->bind_ip, m_pnode_container[i]->nbind_port, \
			m_pnode_container[i]->vnode_list.size());
	}
	return 0;
}
#endif //_DEBUG

CVnodeContainer::CVnodeContainer(int vnode_count, int replica_count) : \
	m_vnode_count(vnode_count), m_replica_count(replica_count)
{
	int vnode_curr;

	try
	{
		m_pvnode_container = new vnode_info_ptr[m_vnode_count];
	}
	catch(std::bad_alloc)
	{
		m_pvnode_container = NULL;
	}
	if(m_pvnode_container == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode container", \
			__LINE__);
		throw ENOMEM;
	}

	for(vnode_curr = 0; vnode_curr < m_vnode_count; vnode_curr++)
	{
		try
		{
			m_pvnode_container[vnode_curr] = new vnode_info();
		}
		catch(std::bad_alloc)
		{
			m_pvnode_container[vnode_curr] = NULL;
		}
		if(m_pvnode_container[vnode_curr] == NULL)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"no more memory to create a vnode info object");
			throw ENOMEM;
		}
	}

	try
	{
		m_pvnode_rwlock = new CRWLock();
	}
	catch(std::bad_alloc)
	{
		m_pvnode_rwlock = NULL;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"call CRWLock constructor failed, err: %s", \
			__LINE__, strerror(errcode));
		throw errcode;
	}
	if(m_pvnode_rwlock == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create vnode rwlock", \
			__LINE__);
		throw ENOMEM;
	}
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

	pvnode_info = m_pvnode_container[index];
	if(pvnode_info == NULL || pvnode_info->vnode_id != index)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"the vnode info(index: %d) is illegal", \
			__LINE__, index);
		return NULL;
	}
	return pvnode_info;
}

int CVnodeContainer::get_vnode_count()
{
	return m_vnode_count;
}

int CVnodeContainer::merge_vnode(vnode_info_ptr pvnode_info)
{
	int res;
	int index;
#ifdef _DEBUG
	assert(pvnode_info);
#endif //_DEBUG
	index = pvnode_info->vnode_id;
	if(index < 0 || index >= m_vnode_count)
	{
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

int CVnodeContainer::destroy_all_replicas_info(device_info_ptr pnode_info)
{
	int i;
	int vnode_index;
	vnode_info_ptr pvnode_info;
	
	while(pnode_info->vnode_list.size() > 0)
	{
		vnode_index = pnode_info->vnode_list.back();
		pnode_info->vnode_list.pop_back();
		pvnode_info = get_vnode(vnode_index);
		pvnode_info->destroy_replica_info(pnode_info);
	}
	return 0;
}