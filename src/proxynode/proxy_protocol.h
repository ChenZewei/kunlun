#ifndef KL_PROXY_PROTOCOL_H_
#define KL_PROXY_PROTOCOL_H_

#include "common_types.h"
#include "common_protocol.h"
/*
* @brief: proxynode-storagenode procotol or proxynode-client procotol
*/
#define KL_PROXY_CMD_DEVICE_JOIN	1
#define KL_PROXY_CMD_BEAT_HEART		2
#define KL_PROXY_CMD_MASTER_RESP	3
#define KL_PROXY_CMD_SLAVER_RESP	4
#define KL_PROXY_CMD_SYNC_DOWN		5 //sync complete
/*
* @brief: storagenode(vnode replica) status
*/
#define KL_REPLICA_STATUS_ONLINE	1
#define KL_REPLICA_STATUS_WAIT_SYNC	2 //synchronized destination replica status
#define KL_REPLICA_STATUS_COPY_SRC	3 //synchronized source replica status
#define KL_REPLICA_STATUS_MOVE_SRC	4
#define KL_REPLICA_STATUS_ACTIVE	5 //service to response all writing and reading requestion 
#define KL_REPLICA_STATUS_OFFLINE	6 //offline status

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	typedef struct _device_join_body
	{
		byte device_bind_ip[KL_COMMON_IP_ADDR_LEN];
		byte device_bind_port[4]; //int variable
		byte zone_id[4];
		byte weight[4];
	}device_join_body, *pdevice_join_body;

	//every device beat heart package is composed of storage_info
	//and a vnode_list
	typedef struct _storage_info
	{
		byte device_bind_ip[KL_COMMON_IP_ADDR_LEN];
		byte device_bind_port[4];
		//byte device_vnode_count[4];
	}storage_info, *pstorage_info;

	typedef struct _vnode_list_unit
	{
		byte vnode_id[4];
		byte vnode_version[8];
		byte vnode_status;
	}vnode_list_unit, *pvnode_list_unit;
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //KL_PROXY_PROTOCOL_H_