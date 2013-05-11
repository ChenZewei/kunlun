#ifndef KL_PROXY_MSG_PARSER_H_
#define KL_PROXY_MSG_PARSER_H_
#include "node_info.h"
#include "msg_parser.h"
#include "timed_stream.h"
#include "proxy_protocol.h"
class CProxyMsgParser : public CMsgParser
{
public:
	CProxyMsgParser();
	~CProxyMsgParser();

	virtual int parse_msg(pkg_message* pkg_msg_ptr);

	int msg_device_join_handle(pkg_message* pkg_msg_ptr);
	int msg_device_report_handle(pkg_message* pkg_msg_ptr);
private:
	/*
	 * @brief: master do device beat-hearting info merged
	 */
	int master_do_device_merge(device_info_ptr pdevice_info, pvnode_list_unit pvnode_list, \
		int vnode_count, CTimedStream *presp_stream);
	/*
	 * @brief: slaver do device beat-hearting info merged
	 */
	int slaver_do_device_merge(device_info_ptr pdevice_info, pvnode_list_unit pvnode_list, \
		int vnode_count);
};
#endif //KL_PROXY_MSG_PARSER_H_