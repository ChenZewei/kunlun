#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "node_info.h"
#include "common_types.h"

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
#ifdef _DEBUG
	KL_SYS_DEBUGLOG("call CDeviceContainer destructor successfully");
#endif //_DEBUG
}

int CDeviceContainer::node_diff_cmp(device_info_ptr ptr1, device_info_ptr ptr2)
{
	if((memcmp(ptr1->bind_ip, ptr2->bind_ip, KL_COMMON_IP_ADDR_LEN) == 0) \
		&& (ptr1->bind_port == ptr2->bind_port))
		return 0;
	return -1;
}

int CDeviceContainer::node_vol_cmp(device_info_ptr ptr1, device_info_ptr ptr2)
{
	float node1_volume;
	float node2_volume;
	
	if(ptr1->weight <= 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the device's(node id: %d) weight is illegal", \
			__LINE__, ptr1->device_id);
		ptr1->weight = 1;
	}
	if(ptr2->weight <= 0)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the device's(node id: %d) weight is illegal", \
			__LINE__, ptr2->device_id);
		ptr2->weight = 1;
	}
	node1_volume = (float)(ptr1->vnode_count) / (float)(ptr1->weight);
	node2_volume = (float)(ptr2->vnode_count) / (float)(ptr2->weight);

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
	return 0;
}

int CDeviceContainer::add_node(device_info_ptr pdevice_info)
{
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