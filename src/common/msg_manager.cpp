#include <stdio.h>
#include <utility>
#include "log.h"
#include "msg_handler.h"
#include "msg_manager.h"

CMsgManager::CMsgManager(CMsgHandler *pmsg_handler) : \
	m_pmsg_handler(NULL), m_callback_map()
{
}

CMsgManager::~CMsgManager()
{
	if(m_pmsg_handler != NULL)
	{
		delete m_pmsg_handler;
		m_pmsg_handler = NULL;
	}
}

int CMsgManager::register_msg(byte msg_id, callback_ptr callback)
{
#define hash_map_pair(a, b) std::pair<byte, byte>(a, b)
	m_callback_map.insert(hash_map_pair(msg_id, callback));
	return 0;
}

int CMsgManager::despatch_msg(pkg_message* pkg_msg_ptr)
{
	byte msg_id;
	callback_ptr callback;
	msg_id = *(pkg_msg_ptr->pkg_ptr);
	typedef byte_hash_map::iterator byte_hash_iter;
	byte_hash_iter b_hash_iter = m_callback_map.find(msg_id);
	callback = (callback_ptr)(b_hash_iter->second);
	if(callback == NULL)
	{
		KL_SYS_ERRLOG("file: "__FILE__", line: %d, " \
			"msg(cmd = %d) has not been registered", \
			__LINE__, msg_id);
		return -1;
	}
	return (m_pmsg_handler->*callback)(pkg_msg_ptr);
}