#ifndef KL_COMMON_MSG_PARSER_H_
#define KL_COMMON_MSG_PARSER_H_
#include "common_protocol.h"
class CMsgParser
{
public:
	CMsgParser();
	virtual ~CMsgParser();

	virtual int parse_msg(pkg_message* pkg_msg_ptr);
};

#endif //KL_COMMON_MSG_PARSER_H_