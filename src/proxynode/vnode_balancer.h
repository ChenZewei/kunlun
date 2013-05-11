#ifndef KL_PROXY_VNODE_BALANCER_H_
#define KL_PROXY_VNODE_BALANCER_H_
#include "node_info.h"
class CVnodeBalancer
{
public:
	CVnodeBalancer();
	~CVnodeBalancer();

	/*
	 * @brief: do device joining operation when the count of node less than
	           the count of replica
	 * @param: pdevice_info, the device info will be joined to node container
	 * @return: if successed, return 0, otherwise, return -1
	 */
	int master_do_copy_vnode(device_info_ptr pdevice_info);
	/*
	 * @brief: do device joining operation when the count of node larger than
	           the count of replica
	 * @param: pdevice_info, the device info will be joined to node container
	 * @return: if successed, return 0, otherwise, return -1
	 */
	int master_do_move_vnode(device_info_ptr pdevice_info);
	/*
	 * @brief: slaver do device joining operation, only add device info to device container
	           not move vnode to the joining device
	 */
	int slaver_do_device_join(device_info_ptr pdevice_info);
};
#endif //KL_PROXY_VNODE_BALANCER_H_