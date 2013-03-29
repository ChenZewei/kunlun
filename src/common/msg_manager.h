#ifndef KL_COMMON_MSG_Manager_H_
#define KL_COMMON_MSG_Manager_H_
class CMsgHandler;
#ifdef __GNUC__
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#include "common_protocol.h"
namespace gnuc
{
	using namespace __gnu_cxx;
}
typedef int (CMsgHandler::*callback_ptr)(pkg_message*);
typedef gnuc::hash_map<unsigned char, callback_ptr> byte_hash_map;
class CMsgManager
{
public:
	/*
	 * @param: pmsg_handler, must be create on heap and deleted by msg manager
	 */
	CMsgManager(CMsgHandler *pmsg_handler);
	~CMsgManager();

	int register_msg(byte msg_id, callback_ptr callback);
	int despatch_msg(pkg_message* pkg_msg_ptr);
private:
	CMsgHandler *m_pmsg_handler;
	byte_hash_map m_callback_map;
};

#endif //KL_COMMON_MSG_Manager_H_