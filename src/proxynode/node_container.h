#ifndef KL_PROXY_NODE_CONTAINER_H_
#define KL_PROXY_NODE_CONTAINER_H_
/*
 * @description: in the kunlun destributed file system, the real storage node
                 is named with device, and the virtual storage node is named
				 with vnode. what's more, kunlun also use zone to limit
				 different replicas of a file being save to the same zone
 */
#include <sys/types.h>
#include "node_info.h"

#define KL_PROXY_CONTAINER_SIZE 100 /* define device node container size */

class CRWLock;
class CDeviceContainer
{
public:
	CDeviceContainer();
	~CDeviceContainer();
	
	/*
	 * @param: pdevice_info, must be created on heap(use operator new)
	           and deleted by CDeviceContainer
	 */
	int add_node(device_info_ptr pdevice_info);
	/*
	 * @param: pdevice_info, CDeviceContainer isn't sure to release the
	           memory hold by pdevice_info
	 * @description: the function merge the node info to node container, 
	                 but it can't create a node in node container, if the
					 merged node isn't in container, the function will return
					 -1, if merge node successfully, the function will return 0
	 */
	int merge_node(device_info_ptr pdevice_info);
	/*
	 * @param: pdevice_info, the function will use the ip info and port info in
	           pdevice_info to make sure the node that will be deleted in node 
			   container, CDeviceContainer isn't sure to release the memory holding
			   by pdevice_info
	 */
	int delete_node(device_info_ptr pdevice_info);
	/*
	 * @param: expected_zones, a list structure, the function will return a device node
	           info expected in these zones
	 * @param: count, the size of expected_zones
	 * @description: the function will return the node info in the head node container,
	                 the function will sort the node container with qsort to make sure
					 return a node that it's volume is the least
	 */
	device_info_ptr get_node(int *expected_zones, int count);
	/*
	 * @brief: get node by node index
	 * @param: index, the node index in node container
	 * @return: if successed, return  the pointer of node, otherwise, return NULL
	 */
	device_info_ptr get_node(int index);
	/*
	 * @brief: get online device count
	 */
	int get_node_count();
	/*
	 * @brief: get the node container's total online replica count and total replica weight
	 * @return: if successed, return 0, if failed, return -1
	 */
	int get_total_replica_count(int *ptotal_count, int *ptotal_weight);

	CRWLock *m_pnode_rwlock;
private:
	/*
	 * @description: compare the two nodes by their volume, if ptr1's volume
	                 is larger than ptr2, the function will return 1, if ptr1's
					 volume equal to ptr2, the function will return 0, if ptr1's
					 volume is smaller than ptr2, the function will return -1
	 */
	static int node_vol_cmp(const void *ptr1, const void *ptr2);
	/*
	 * @description: compare two nodes by their ip address and port address,
	                 if they have the same ip and port, the function will return
					 0, otherwise, the function will return -1
	 */
	int node_diff_cmp(device_info_ptr ptr1, device_info_ptr ptr2);

	device_info_ptr *m_pnode_container;
	int m_node_count;
	int m_container_size;
};

class CVnodeContainer
{
public:
	CVnodeContainer(int vnode_count, int replica_count);
	~CVnodeContainer();
	/*
	 * @brief: must be called after creating a CVnodeContainer obj
	 */
	int initilize();
	/*
	 * @brief: merge vnode info to the specified vnode in vnode container
	 * @param: pvnode_info, the vnode info that will be merged
	 * @return: success, return 0, if can't locate the specified vnode in vnode
	            container, return -1
	 */
	int merge_vnode(vnode_info_ptr pvnode_info);
	/*
	 * @brief: get the vnode info by the specified vnode index
	 * @param: index, the vnode index in container
	 * @return: return the pointer of the specified vnode
	 */
	/*
	 * @brief: destroy all replicas info about the specified storagenode
	 * @param: pnode_info, to specify the destroy
	 * @return: always return 0
	 */
	int destroy_all_replicas_info(device_info_ptr pnode_info);
	vnode_info_ptr get_vnode(int index);
	int get_vnode_count();
	int get_replica_count();

	CRWLock *m_pvnode_rwlock;
private:
	int m_vnode_count;
	int m_replica_count;
	vnode_info_ptr *m_pvnode_container;
};
#endif //KL_PROXY_NODE_CONTAINER_H_