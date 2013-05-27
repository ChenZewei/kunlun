#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "directory.h"
#include "tb_account.h"
#include "vnode_info.h"
#include "tb_hashcode.h"
#include "common_types.h"
#include "tb_container.h"
#include "storage_global.h"
#include "tb_delete_file_record.h"

storage_vnode::storage_vnode() : vnode_id(0), \
	vnode_status(0), vnode_version(0)
{
}

storage_vnode::~storage_vnode()
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

CStorageVnodeContainer::CStorageVnodeContainer()
{
}

CStorageVnodeContainer::~CStorageVnodeContainer()
{
	while(!m_vnode_list.empty())
	{
		delete m_vnode_list.front();
		m_vnode_list.pop_front();
	}
}

storage_vnode* CStorageVnodeContainer::at(int index)
{
	if(index >= m_vnode_list.size() || index < 0)
		return NULL;

	int i = 0;
	vnode_list_iter_t vnode_iter;

	vnode_iter = m_vnode_list.begin();
	while(i < index)
	{
		i++;
		vnode_iter++;
	}
	return *vnode_iter;
}

storage_vnode* CStorageVnodeContainer::get_vnode(int vnode_id)
{
	vnode_list_iter_t vnode_iter;
	for(vnode_iter = m_vnode_list.begin(); vnode_iter != m_vnode_list.end(); \
		vnode_iter++)
	{
		if((*vnode_iter)->vnode_id == vnode_id)
			return *vnode_iter;
	}
	//not found
	return NULL;
}

int CStorageVnodeContainer::add_vnode(storage_vnode* pvnode_info)
{
	if(pvnode_info == NULL)
		return -1;

	CDirectory dir;
	int vnode_id, ret;
	char file_path[KL_COMMON_PATH_LEN];
	char vnode_dir[KL_COMMON_PATH_LEN];

	m_vnode_list.push_back(pvnode_info);
	vnode_id = pvnode_info->vnode_id;
	//to create dir and database
	try
	{
		//create object dir
		memset(vnode_dir, 0, KL_COMMON_PATH_LEN);
		snprintf(vnode_dir, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/object", \
			g_device_root, vnode_id);
		if((ret = dir.make_dir(vnode_dir)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"create dir failed, dir_path: %s, err: %s", \
				__LINE__, vnode_dir, strerror(ret));
			return ret;
		}
		//create hashcode database
		memset(file_path, 0, KL_COMMON_PATH_LEN);
		snprintf(file_path, KL_COMMON_PATH_LEN, "%s/hashcode.db", vnode_dir);
		CTbHashCode tb_hash_code(file_path);
		//create delete file record database
		memset(file_path, 0, KL_COMMON_PATH_LEN);
		snprintf(file_path, KL_COMMON_PATH_LEN, "%s/delete_file_record.db", vnode_dir);
		CTbDeleteFileRecord tb_delete_file_record(file_path);
		//create account dir
		memset(vnode_dir, 0, KL_COMMON_PATH_LEN);
		snprintf(vnode_dir, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/account", \
			g_device_root, vnode_id);
		if((ret = dir.make_dir(vnode_dir)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"create dir failed, dir_path: %s, err: %s", \
				__LINE__, vnode_dir, strerror(ret));
			return ret;
		}
		//create account database
		memset(file_path, 0, KL_COMMON_PATH_LEN);
		snprintf(file_path, KL_COMMON_PATH_LEN, "%s/account.db", vnode_dir);
		CTbAccount tb_account(file_path);
		//create container dir
		memset(vnode_dir, 0, KL_COMMON_PATH_LEN);
		snprintf(vnode_dir, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d/container", \
			g_device_root, vnode_id);
		if((ret = dir.make_dir(vnode_dir)) != 0)
		{
			KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
				"create dir failed, dir_path: %s, err: %s", \
				__LINE__, vnode_dir, strerror(ret));
			return ret;
		}
		//create container database
		memset(file_path, 0, KL_COMMON_PATH_LEN);
		snprintf(file_path, KL_COMMON_PATH_LEN, "%s/container.db", vnode_dir);
		CTbContainer tb_container(file_path);
	}
	catch(std::bad_alloc)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create object database", \
			__LINE__);
		return ENOMEM;
	}
	catch(int errcode)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create object database failed, err: %s", \
			__LINE__, strerror(errcode));
		ret = errcode != 0 ? errcode : ENOMEM;
		return ret;
	}
	catch(const char *perrmsg)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"create object database failed, err: %s", \
			__LINE__, perrmsg);
		return ENOMEM;
	}
	return 0;
}

int CStorageVnodeContainer::delete_vnode(int vnode_id)
{
	int ret;
	CDirectory dir;
	vnode_list_iter_t vnode_iter;
	char vnode_dir[KL_COMMON_PATH_LEN];

	for(vnode_iter = m_vnode_list.begin(); vnode_iter != m_vnode_list.end(); \
		vnode_iter++)
	{
		if((*vnode_iter)->vnode_id == vnode_id)
		{
			delete (*vnode_iter);
			(*vnode_iter) = NULL;
			m_vnode_list.erase(vnode_iter);
			break;
		}
	}
	memset(vnode_dir, 0, KL_COMMON_PATH_LEN);
	//remove the dir of vnode
	snprintf(vnode_dir, KL_COMMON_PATH_LEN, "%s/"KL_STORAGE_VNODE_NAME_PREFIX"%d", \
		g_device_root, vnode_id);
	dir.remove_dir(vnode_dir);
	return 0;
}

int CStorageVnodeContainer::get_vnode_count()
{
	return m_vnode_list.size();
}