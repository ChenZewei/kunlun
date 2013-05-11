#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vnode_info.h"

storage_vnode::storage_vnode() : vnode_id(0), \
	vnode_status(0), vnode_version(0)
{
}

storage_vnode::~storage_vnode()
{
}

CStorageVnodeContainer::CStorageVnodeContainer()
{
}

storage_sync_event::storage_sync_event() : \
	vnode_id(0), sync_dest_port(0)
{
	memset(sync_dest_ip, 0, KL_COMMON_IP_ADDR_LEN);
}

storage_sync_event::~storage_sync_event()
{

}

CStorageVnodeContainer::~CStorageVnodeContainer()
{
}

storage_vnode* CStorageVnodeContainer::at(int index)
{
	return m_vnode_list.at(index);
}

storage_vnode* CStorageVnodeContainer::getvnode(int vnode_id)
{
	int i;
	for(i = 0; i < m_vnode_list.size(); i++)
	{
		if(m_vnode_list[i]->vnode_id == vnode_id)
			return m_vnode_list[i];
	}
	//not found
	return NULL;
}

int CStorageVnodeContainer::add_vnode(storage_vnode* pvnode_info)
{
	if(pvnode_info == NULL)
		return -1;
	m_vnode_list.push_back(pvnode_info);
	return 0;
}

int CStorageVnodeContainer::delete_vnode(int vnode_id)
{
	int i;
	for(i = 0; i < m_vnode_list.size(); i++)
	{
		if(m_vnode_list[i]->vnode_id == vnode_id)
		{
			delete m_vnode_list[i];
			m_vnode_list[i] = NULL;
			m_vnode_list.erase(m_vnode_list.begin() + i);
		}
	}
	return 0;
}

int CStorageVnodeContainer::get_vnode_count()
{
	return m_vnode_list.size();
}