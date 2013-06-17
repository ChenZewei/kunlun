#ifndef KL_PROXY_MSG_PARSER_H_
#define KL_PROXY_MSG_PARSER_H_
#include "node_info.h"
#include "msg_parser.h"
#include "timed_stream.h"
#include "proxy_protocol.h"
class CProxyMsgParser : public CMsgParser
{
#define msg_read_account_handle(pkg_msg_ptr) msg_get_vnode_addr_handle(pkg_msg_ptr, true)
#define msg_read_container_handle(pkg_msg_ptr) msg_get_vnode_addr_handle(pkg_msg_ptr, true)
#define msg_read_file_handle(pkg_msg_ptr) msg_get_vnode_addr_handle(pkg_msg_ptr, true)
#define msg_write_account_handle(pkg_msg_ptr) msg_get_vnode_addr_handle(pkg_msg_ptr, false)
#define msg_write_container_handle(pkg_msg_ptr) msg_get_vnode_addr_handle(pkg_msg_ptr, false)
#define msg_write_file_handle(pkg_msg_ptr) msg_get_vnode_addr_handle(pkg_msg_ptr, false)

public:
	CProxyMsgParser();
	~CProxyMsgParser();

	virtual int parse_msg(pkg_message *pkg_msg_ptr);
private:
	int msg_device_join_handle(pkg_message *pkg_msg_ptr);
	int msg_device_report_handle(pkg_message *pkg_msg_ptr);
	int msg_sync_down_handle(pkg_message *pkg_msg_ptr);
	int msg_get_vnode_addr_handle(pkg_message *pkg_msg_ptr, bool bread_flag);
	/*
	 * @brief: master do device beat-hearting info merged
	 */
	int master_do_device_merge(device_info_ptr pdevice_info, pvnode_report_info pvnode_list, \
		int vnode_count, CTimedStream *presp_stream);
	/*
	 * @brief: slaver do device beat-hearting info merged
	 */
	int slaver_do_device_merge(device_info_ptr pdevice_info, pvnode_report_info pvnode_list, \
		int vnode_count);
};
#endif //KL_PROXY_MSG_PARSER_H_