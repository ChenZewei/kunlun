#ifndef KL_STORAGE_MSG_PARSER_H_
#define KL_STORAGE_MSG_PARSER_H_
#include "msg_parser.h"
#include "proxy_protocol.h"
class CStorageMsgParser : public CMsgParser
{
public:
	CStorageMsgParser();
	~CStorageMsgParser();

	virtual int parse_msg(pkg_message* pkg_msg_ptr);
};
#endif //KL_STORAGE_MSG_PARSER_H_