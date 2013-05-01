#ifndef KL_PROXY_NODE_INFO_H_
#define KL_PROXY_NODE_INFO_H_

#include <queue>
#include <vector>
#include <stdint.h>
#include "common_types.h"
#include "common_protocol.h"

class device_info;
typedef device_info *device_info_ptr;

class sync_event
{
public:
	int src_vnode_index;
	device_info_ptr pdest_node;
};
typedef sync_event *sync_event_ptr;

class device_info
{
public:
	int erase_vnode(int vnode_index);

	//deprecated device_id
	int device_id; //sign the device in the storage node cluster(online)(not use)
	int zone_id;
	int weight;
	char bind_ip[KL_COMMON_IP_ADDR_LEN];
	int bind_port;
	time_t last_update_time;
	std::vector<int> vnode_list; //vnode_list, used to save vnode replica
	std::queue<sync_event> sync_queue;
};

class replica_info
{
public:
	byte replica_status;
	device_info_ptr preplica;
};
typedef replica_info *replica_info_ptr;

class vnode_info
{
public:
	~vnode_info();
	/*
	 * @brief: get a replica to write data
	 * @param: index, the vnode index
	 * @param: ppreplica, if get replica successfully, the function will set the pointer
	           to the choosen replica, otherwise, set the pointer to null
	 * @return: if call successfully, return 0, if has no active replica, return -1, 
	            otherwise, return -2
	 */
	int get_write_replica(replica_info_ptr *ppreplica);
	/*
	 * @brief: get a replica to read data
	 * @param: index, the vnode index
	 * @param: ppreplica, if get replica successfully, the function will set the pointer
	           to the choosen replica, otherwise, set the pointer to null
	 * @return: if call successfully, return 0, if has no active replica, return -1, 
	            otherwise, return -2
	 */
	int get_read_replica(replica_info_ptr *ppreplica);
	/*
	 * @brief: get the specified replica's status
	 * @param: preplica, specify replica
	 * @return: if successed, return the replica pointer, if failed, return NULL
	 */
	replica_info_ptr get_replica_info(device_info_ptr preplica);
	/*
	 * @brief: test the joining device whether located in different zones with vnode's replicas
	 * @param: pjoin_device, the device info that will be test
	 * @return if locating in different zones, return true, otherwise, return false;
	 */
	bool is_diff_zones(device_info_ptr pjoin_device);
	/*
	 * @brief: destroy the specified replica info
	 * @param: preplica, the specified replica
	 * @return: always return 0
	 */
	int destroy_replica_info(device_info_ptr preplica);

	//vnode_id will not be changed after vnode being intilized
	//when version wr_req_robin rd_req_robin replica_list's status be changed,
	//we must use vnode rwlock to make sure the consistency of data
	int vnode_id;
	//	int64_t key_hash; //key_hash divide the name space of kunlun to equivalent parts
	int64_t version; //vnode version
	int wr_req_robin;
	int rd_req_robin;
	//replica_list's size changing only occurred when device join or exit
	std::vector<replica_info_ptr> replica_list; 
};
typedef vnode_info *vnode_info_ptr;
#endif //KL_PROXY_NODE_INFO_H_