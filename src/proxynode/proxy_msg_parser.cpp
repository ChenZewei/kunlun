#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "node_info.h"
#include "sockstream.h"
#include "proxy_global.h"
#include "proxy_protocol.h"
#include "proxy_msg_parser.h"
#ifdef _DEBUG
#include <assert.h>
#endif //_DEBUG

CProxyMsgParser::CProxyMsgParser()
{
}

CProxyMsgParser::~CProxyMsgParser()
{
}

int CProxyMsgParser::parse_msg(pkg_message* pkg_msg_ptr)
{
#ifdef _DEBUG
	assert(pkg_msg_ptr);
#endif //_DEBUG
	byte msg_cmd;
	if(pkg_msg_ptr->pkg_ptr == NULL)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the context of message package is null", \
			__LINE__);
		return -1;
	}

	msg_cmd = *(pkg_msg_ptr->pkg_ptr);
	switch(msg_cmd)
	{
	case KL_PROXY_CMD_DEVICE_JOIN:
		return msg_device_join_handle(pkg_msg_ptr);
	case KL_PROXY_CMD_BEAT_HEART:
		return msg_device_beat_heart_handle(pkg_msg_ptr);
	default:
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"catch undefined msg...", \
			__LINE__);
		return -1;
	}
}

int CProxyMsgParser::msg_device_join_handle(pkg_message* pkg_msg_ptr)
{
	int res;
	int zone_id;
	int weight;
	int device_bind_port;
	char *pdevice_bind_ip;
	pdevice_join_body pbody;
	device_info_ptr pdevice_info;

	if(sizeof(device_join_body) != pkg_msg_ptr->pkg_len - 2)
	{
		KL_SYS_WARNNINGLOG("file: "__FILE__", line: %d, " \
			"the msg pkg(msg_cmd = %d) of device join is illegal", \
			__LINE__, pkg_msg_ptr->pkg_ptr);
		delete pkg_msg_ptr;
		return -1;
	}

	pbody = (pdevice_join_body)((pkg_msg_ptr->pkg_ptr) + 2);
	pdevice_bind_ip = (char*)(pbody->device_bind_ip);
	device_bind_port = CSERIALIZER::buff2int32(pbody->device_bind_port);
	zone_id = CSERIALIZER::buff2int32(pbody->zone_id);
	weight = CSERIALIZER::buff2int32(pbody->weight);

	pdevice_info = new device_info();
	if(pdevice_info == NULL)
	{
		KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
			"no more memory to create a device info obj", \
			__LINE__);
		delete pkg_msg_ptr;
		return ENOMEM;
	}
	memcpy(pdevice_info->bind_ip, pdevice_bind_ip, KL_COMMON_IP_ADDR_LEN);
	pdevice_info->bind_port = device_bind_port;
	pdevice_info->device_id = 0;
	pdevice_info->vnode_count = 0;
	pdevice_info->weight = weight;
	pdevice_info->zone_id = zone_id;

	do{
		int vnode_index;
		if(g_pdevice_container->get_node_count() < \
			g_pvnode_container->get_replica_count())
		{
			if((res = m_device_join_mutex.lock()) != 0)
			{
				KL_SYS_ERRORLOG("file: "__FILE__", line: %d, " \
					"lock the m_device_join_mutex failed, err: %s", \
					__LINE__, strerror(res));
				break;
			}
			//each vnode does not have enough replicas
			if(g_pdevice_container->get_node_count() < \
				g_pvnode_container->get_replica_count())
			{
				for(vnode_index = 0; vnode_index < )
			}
		}
	}while(0);

	delete pkg_msg_ptr;
	return res;
}