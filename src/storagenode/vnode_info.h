#ifndef KL_STORAGE_VNODE_INFO_H_
#define KL_STORAGE_VNODE_INFO_H_
#include <vector>
#include <stdint.h>
#include "common_protocol.h"
class storage_vnode
{
public:
	storage_vnode();
	~storage_vnode();

	int vnode_id;
	byte vnode_status;
	int64_t vnode_version;
};

class CStorageVnodeContainer
{
public:
	CStorageVnodeContainer();
	~CStorageVnodeContainer();

	/*
	 * @brief: get vnode info by vnode index in vnode container
	 * @param: index in vnode container
	 * @return: return the vnode info
	 */
	storage_vnode* at(int index);
	storage_vnode* getvnode(int vnode_id);
	/*
	 * @brief: add a vnode info to vnode container
	 * @param: pvnode_info, must create on heap
	 * @return: if successful, return 0, otherwise, return -1
	 */
	int add_vnode(storage_vnode* pvnode_info);
	int delete_vnode(int vnode_id);
	int get_vnode_count();

private:
	std::vector<storage_vnode*> m_vnode_list;
};
#endif //KL_STORAGE_VNODE_INFO_H_