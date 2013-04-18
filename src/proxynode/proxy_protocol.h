#ifndef KL_PROXY_PROTOCOL_H_
#define KL_PROXY_PROTOCOL_H_

#include "common_types.h"
#include "common_protocol.h"
/*
 * @brief: proxynode-storagenode procotol or proxynode-client procotol
 */
#define KL_PROXY_CMD_DEVICE_JOIN	1
#define KL_PROXY_CMD_BEAT_HEART		2
#define KL_PROYX_CMD_MASTER_RESP	3
#define KL_PROXY_CMD_SLAVER_RESP	4
/*
 * @brief: storagenode(vnode replica) status
 */
#define KL_DEVICE_STATUS_INIT		0
#define KL_DEVICE_STATUS_WAIT_SYNC	1
#define KL_DEVICE_STATUS_SYNCING	2
#define KL_DEVICE_STATUS_ACTIVE		3
typedef struct _device_join_body
{
	byte device_bind_ip[KL_COMMON_IP_ADDR_LEN];
	byte device_bind_port[4]; //int variable
	byte zone_id[4];
	byte weight[4];
}device_join_body, *pdevice_join_body;

//every device beat heart package is composed of device_beat_heart_body
//and a vnode_list
typedef struct _device_beat_heart_body
{
	byte device_bind_ip[KL_COMMON_IP_ADDR_LEN];
	byte device_bind_port[4];
	byte device_vnode_count[4];
}device_beat_heart_body, *pdevice_beat_heart_body;

typedef struct _vnode_list_unit
{
	byte vnode_version[8];
}vnode_list_unit, *pvnode_list_unit;

#endif //KL_PROXY_PROTOCOL_H_