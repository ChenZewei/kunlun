#ifndef KL_PROXY_MSG_PARSER_H_
#define KL_PROXY_MSG_PARSER_H_
#include "mutex.h"
#include "msg_parser.h"
class CProxyMsgParser : CMsgParser
{
public:
	CProxyMsgParser();
	~CProxyMsgParser();

	virtual int parse_msg(pkg_message* pkg_msg_ptr);

	int msg_device_join_handle(pkg_message* pkg_msg_ptr);
	int msg_device_beat_heart_handle(pkg_message* pkg_msg_ptr);
private:
	CMutex m_device_join_mutex;
};
#endif //KL_PROXY_MSG_PARSER_H_